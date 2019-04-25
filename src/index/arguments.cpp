#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <exception>
#include <stdexcept>

#include "arguments.h"

const std::vector<Arguments::Param> Arguments::params = {
	Param("--help", "-h", "print info about arguments", [](const Arguments& args, const std::string) {
		args.printHelp();
		exit(0);
	}),

	Param("--M", "max count of neighbours for nodes on non-zero layers",
		[](const Arguments &args, const std::string value) {args.indexSettings.M = args.positive(std::stoi(value));}),

	Param("--M0", "max count of neighbours for nodes on zero layers",
		[](const Arguments &args, const std::string value) {args.indexSettings.M0 = args.positive(std::stoi(value));}),

	Param("--efConstruction", "-eC", "count of tracked nearest nodes during index creation",
		[](const Arguments &args, const std::string &value) {args.indexSettings.efConstruction = args.positive(std::stoi(value));}),

	Param("--efSearch", "-eS", "count of tracked nearest nodes during search",
		[](const Arguments &args, const std::string &value) {args.indexSettings.efSearch = args.positive(std::stoi(value));}),

	Param("--mL", "prefactor for random level generation",
		[](const Arguments &args, const std::string &value) {args.indexSettings.mL = args.positiveOrZero(std::stod(value));}),

	Param("--keepPrunedConnections", "-k", "if to keep constant number of nodes neighbours",
		[](const Arguments &args, const std::string&) {args.indexSettings.keepPrunedConnections = true;}),

	Param("--data", "path to file with objects for index",
		[](const Arguments &args, const std::string &value) {args.dataPath = args.notEmpty(value);}),

	Param("--dump", "path to file with index dump",
		[](const Arguments &args, const std::string &value) {args.dumpPath = args.notEmpty(value);}),

	Param("--base", "count of object, that will be inserted sequentially",
		[](const Arguments &args, const std::string &value) {args.baseSize = args.positiveOrZero(std::stoi(value));}),

	Param("--port", "port, that web-server listen to",
		[](const Arguments &args, const std::string &value) {args.port = args.positiveOrZero(std::stoi(value));}),
};

template<class T>
T Arguments::positive(T value) const {
	if (value <= 0) {
		throw std::exception("value should be positive");
	}

	return value;
}

template<class T>
T Arguments::positiveOrZero(T value) const {
	if (value < 0) {
		throw std::exception("value should be positive or zero");
	}

	return value;
}

std::string Arguments::notEmpty(std::string value) const {
	if (value.empty()) {
		throw std::exception("value shouldn't be empty");
	}

	return value;
}

Arguments::Arguments(int argc, char **argv) {
	for (int i = 1; i < argc; ++i) {
		std::istringstream argStream(argv[i]);

		std::string paramName;
		std::string value;

		getline(argStream, paramName, '=');
		getline(argStream, value);

		bool isHandled = false;
		std::string errorMessage;

		for (const Param &param : params) {
			if (param.check(paramName)) {

				try {
					param.handle(*this, value);
				} catch (const std::invalid_argument &e) {
					errorMessage = "invalid value";
				} catch (const std::out_of_range &e) {
					errorMessage = "value is out of range";
				} catch (const std::exception &e) {
					errorMessage = e.what();
				}

				isHandled = true;
				break;
			}
		}

		if (!isHandled) {
			errorMessage = "unknown parameter";
		}

		if (!errorMessage.empty()) {
			throw ArgumentsException(paramName + ": " + errorMessage);
		}
	}
}

void Arguments::printHelp() const {
	for (const Param &param : params) {
		param.print();
	}
}
