#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <functional>
#include <iostream>
#include <iomanip>
#include <exception>

#include "index.h"

class Arguments {
	class Param;

	static const std::vector<Param> params;

	template<class T>
	T positive(T value) const;

	template<class T>
	T positiveOrZero(T value) const;

	std::string notEmpty(std::string value) const;

public:
	mutable Settings indexSettings;
	mutable std::string dataPath = "index.data";
	mutable std::string dumpPath = "index.dump";
	mutable std::string dataset = "./";
	mutable int baseSize = 1000;
	mutable std::string address = "127.0.0.1";
	mutable int port = 8000;

	Arguments(int argc, char **argv);

	void printHelp() const;
};

class Arguments::Param {
	std::string name;
	std::string shortName;
	std::string description;
	std::function<void(const Arguments&, const std::string&)> action;

public:
	Param(
		std::string name,
		std::string shortName,
		std::string description,
		std::function<void(const Arguments&, const std::string&)> action
	) :
		name(std::move(name)),
		shortName(std::move(shortName)),
		description(std::move(description)),
		action(std::move(action)) {}

	Param(
		std::string name,
		std::string description,
		std::function<void(const Arguments&, const std::string&)> action
	) :
		Param(std::move(name), "", std::move(description), std::move(action)) {}

	bool check(const std::string &paramName) const {
		return paramName == name || paramName == shortName;
	}

	void handle(const Arguments &args, const std::string value) const {
		action(args, value);
	}

	void print() const {
		std::cout << std::left << std::setw(5) << shortName <<
			std::left << std::setw(30) << name << description << std::endl;
	}
};

class ArgumentsException : public std::exception {
	std::string message;

public:
	ArgumentsException(std::string message) : message(std::move(message)) {}

	const char* what() const noexcept {
		return message.c_str();
	}
};

#endif
