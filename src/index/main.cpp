#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "index.h"
#include "httplib.h"

std::vector<double> parseDescriptor(std::istream &in) {
	std::vector<double> descriptor;
	std::string item;

	while (getline(in, item, ',')) {
		descriptor.push_back(stod(item));
	}

	return descriptor;
}

Index loadIndex(std::string fileName) {
	// TODO: check file existence

	std::ifstream file(fileName);
	std::string line;

	getline(file, line);

	int descriptorSize = stoi(line);
	Index index(descriptorSize);

	while (getline(file, line)) {
		// TODO: check reading

		std::istringstream lineStream(line);

		std::string name;
		getline(lineStream, name, ',');

		std::vector<double> descriptor = parseDescriptor(lineStream);

		// TODO: check descriptor size

		index.insert(std::move(name), std::move(descriptor));
	}

	return index;
}

int main() {
	std::cout << "Indexing..." << std::endl;
	Index index = loadIndex("index.data");
	
	httplib::Server app;

	app.Get("/health", [](const httplib::Request&, httplib::Response &res) {
		res.set_content("I'm OK", "text/plain");
	});

	app.Post("/neighbour", [&index](const httplib::Request &req, httplib::Response &res) {
		std::istringstream bodyStream(req.body);
		std::vector<double> descriptor = parseDescriptor(bodyStream);

		// TODO: check descriptor size

		SearchResult searchResult = index.search(std::move(descriptor), 1)[0];

		// TODO: handle empty result

		std::ifstream file(searchResult.name, std::ios::binary | std::ios::ate);

		int size = file.tellg();
		char *image = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(image, size);

		res.set_content(image, size, "image/jpeg");
		res.set_header("Name", searchResult.name.c_str());

		// TODO: handle image extension

		file.close();
		delete[] image;
	});

	int port = 8000;
	std::cout << "Server is listening on port " << port << std::endl;
	app.listen("localhost", port);

	return 0;
}
