#include <iostream>
#include <fstream>
#include <sstream>

#include "index.h"

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
	Index index = loadIndex("index.data");

	return 0;
}
