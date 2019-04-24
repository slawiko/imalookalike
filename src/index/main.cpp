#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "index.h"
#include "thread_pool.h"
#include "arguments.h"
#include "httplib.h"

std::vector<double> parseDescriptor(std::istream &in, int descriptorSize) {
	std::vector<double> descriptor;
	descriptor.reserve(descriptorSize);
	std::string item;

	while (getline(in, item, ',')) {
		descriptor.push_back(stod(item));
	}

	return descriptor;
}

void parseAndInsert(std::string line, Index &index, int descriptorSize) {
	static int counter = 0;

	std::istringstream lineStream(line);

	std::string name;
	getline(lineStream, name, ',');

	std::vector<double> descriptor = parseDescriptor(lineStream, descriptorSize);

	index.insert(std::move(name), std::move(descriptor));

	++counter;
	if (counter % 1000 == 0)
		std::cout << counter << std::endl;
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
	std::string line;

	getline(dataFile, line);

	int descriptorSize = stoi(line);
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

void setServerRoutes(httplib::Server &server, Index &index) {
	server.Get("/health", [](const httplib::Request&, httplib::Response &res) {
		res.set_content("I'm OK", "text/plain");
	});

	server.Post("/neighbour", [&index](const httplib::Request &req, httplib::Response &res) {
		std::istringstream bodyStream(req.body);
		std::vector<double> descriptor = parseDescriptor(bodyStream, index.getDescriptorSize());

		SearchResult searchResult = index.search(std::move(descriptor), 1)[0];

		std::ifstream file(searchResult.name, std::ios::binary | std::ios::ate);

		int size = file.tellg();
		char *image = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(image, size);

		res.set_content(image, size, "image/jpeg");
		res.set_header("Name", searchResult.name.c_str());

		file.close();
		delete[] image;
	});
}

int main(int argc, char **argv) {
	Arguments args = parseArguments(argc, argv);

	Index index = createIndex(args.indexSettings, args.dataPath, args.dumpPath, args.baseSize);

	httplib::Server server;
	setServerRoutes(server, index);

	std::cout << "Server is listening on port " << args.port << std::endl;
	server.listen("localhost", args.port);

	return 0;
}
