#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <exception>
#include <stdexcept>
#include <unordered_map>

#include "index.h"
#include "thread_pool.h"
#include "arguments.h"
#include "httplib.h"

std::vector<double> parseDescriptor(std::istream &in, int descriptorSize) {
	std::vector<double> descriptor;
	descriptor.reserve(descriptorSize);
	std::string item;

	try {
		while (getline(in, item, ',')) {
			descriptor.push_back(std::stod(item));
		}
	} catch (const std::invalid_argument &e) {
		throw std::runtime_error("Invalid value");
	} catch (const std::out_of_range &e) {
		throw std::runtime_error("Value is out of range");
	}

	if (descriptor.size() != descriptorSize) {
		throw std::runtime_error("Incorrect descriptor size");
	}

	return descriptor;
}

void parseAndInsert(std::string line, Index &index, int descriptorSize) {
	std::istringstream lineStream(line);

	std::string name;
	getline(lineStream, name, ',');

	std::vector<double> descriptor = parseDescriptor(lineStream, descriptorSize);

	index.insert(std::move(name), std::move(descriptor));
}

Index createIndex(Settings settings, std::string dataPath, std::string dumpPath, int baseSize) {
	std::ifstream dumpFile(dumpPath);

	if (dumpFile.good()) {
		dumpFile.close();

		std::cout << "Reading dump..." << std::endl;

		return Index(dumpPath);
	}

	std::cout << "Indexing..." << std::endl;

	std::ifstream dataFile(dataPath);

	if (dataFile.fail()) {
		throw std::runtime_error("Can't find neither data file nor dump file");
	}

	std::string line;

	getline(dataFile, line);

	int descriptorSize = std::stoi(line);
	Index index(descriptorSize, settings);

	for (int i = 0; i < baseSize && std::getline(dataFile, line); ++i) {
		parseAndInsert(line, index, descriptorSize);
	}

	if (!dataFile.eof()) {
		ThreadPool threadPool;

		while (std::getline(dataFile, line)) {
			threadPool.enqueu([line, &index, descriptorSize]() {
				parseAndInsert(line, index, descriptorSize);
			});
		}

		threadPool.wait();
	}

	index.save(dumpPath);

	return index;
}

std::string pickContentType(std::string fileName) {
	static const std::unordered_map<std::string, std::string> contentTypes = {
		{"jpeg", "image/jpeg"},
		{"jpg", "image/jpeg"},
		{"png", "image/png"},
		{"gif", "image/gif"},
		{"bmp", "image/bmp"},
		{"tiff", "image/tiff"},
		{"tif", "image/tiff"},
	};

	static const std::string defaulContentType = "application/octet-stream";

	int extPos = fileName.find_last_of('.');

	if (extPos == -1) {
		return defaulContentType;
	}

	std::string extension = fileName.substr(extPos + 1);
	auto item = contentTypes.find(extension);

	if (item != contentTypes.end()) {
		return item->second;
	} else {
		return defaulContentType;
	}
}

void setServerRoutes(httplib::Server &server, Index &index, const std::string &dataset) {
	server.Get("/health", [](const httplib::Request&, httplib::Response &res) {
		res.set_content("I'm OK", "text/plain");
	});

	server.Get("/descriptor-size", [&index](const httplib::Request&, httplib::Response &res) {
		res.set_content(std::to_string(index.getDescriptorSize()), "text/plain");
	});

	server.Post("/neighbour", [&index, &dataset](const httplib::Request &req, httplib::Response &res) {
		std::istringstream bodyStream(req.body);
		std::vector<double> descriptor;

		try {
			descriptor = parseDescriptor(bodyStream, index.getDescriptorSize());
		} catch (const std::exception &e) {
			res.status = 400;
			res.set_content(e.what(), "text/plain");
			return;
		}

		std::vector<SearchResult> searchResults = index.search(std::move(descriptor), 1);

		if (searchResults.empty()) {
			res.set_content("Index is empty", "text/plain");
			return;
		}

		SearchResult searchResult = searchResults.front();

		std::string imagePath = dataset + '/' + searchResult.name;
		std::ifstream file(imagePath, std::ios::binary | std::ios::ate);

		if (file.fail()) {
			res.status = 500;
			res.set_content("Can't find an image in the dataset", "text/plain");
			return;
		}

		int size = file.tellg();
		char *image = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(image, size);

		res.set_content(image, size, pickContentType(searchResult.name).c_str());
		res.set_header("Name", searchResult.name.c_str());

		file.close();
		delete[] image;
	});
}

int main(int argc, char **argv) {
	try {
		Arguments args(argc, argv);

		Index index = createIndex(args.indexSettings, args.dataPath, args.dumpPath, args.baseSize);

		httplib::Server server;
		setServerRoutes(server, index, args.dataset);

		std::cout << "Server is listening on " << args.address << ":" << args.port << std::endl;
		if (!server.listen(args.address.c_str(), args.port)) {
			throw std::runtime_error("Invalid address or port");
		}
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
		return -1;
	}

	return 0;
}
