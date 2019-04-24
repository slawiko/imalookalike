#ifndef ARGUMENTS_H
#define ARGIMNETS_H

#include <string>
#include <functional>
#include <iostream>
#include <iomanip>

#include "index.h"

struct Arguments {
	Settings indexSettings;
	std::string dataPath = "index.data";
	std::string dumpPath = "index.dump";
	int baseSize = 1000;
	int port = 8000;
};

class Param {
	std::string name;
	std::string shortName;
	std::string description;
	std::function<void(Arguments&, std::string&)> action;

public:
	Param(std::string name, std::string shortName, std::string description, std::function<void(Arguments&, std::string&)> action) :
		name(std::move(name)), shortName(std::move(shortName)), description(std::move(description)), action(std::move(action)) {}

	Param(std::string name, std::string description, std::function<void(Arguments&, std::string&)> action) :
		Param(std::move(name), "", std::move(description), std::move(action)) {}

	bool check(std::string paramName) const {
		return paramName == name || paramName == shortName;
	}

	void apply(Arguments &args, std::string value) const {
		action(args, value);
	}

	void print() const {
		std::cout << std::left << std::setw(5) << shortName <<
			std::left << std::setw(30) << name << description << std::endl;
	}
};

void printHelp();
Arguments parseArguments(int argc, char **argv);

#endif
