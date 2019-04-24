#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include "arguments.h"

static const std::vector<Param> params{
	Param("--help", "-h", "print info about arguments", [](Arguments&, std::string) {
		printHelp();
		exit(0);
	}),

	Param("--M", "max count of neighbours for nodes on non-zero layers",
		[](Arguments &args, std::string value) {args.indexSettings.M = std::stoi(value);}),

	Param("--M0", "max count of neighbours for nodes on zero layers",
		[](Arguments &args, std::string value) {args.indexSettings.M0 = std::stoi(value);}),

	Param("--efConstruction", "-eC", "count of tracked nearest nodes during index creation",
		[](Arguments &args, std::string &value) {args.indexSettings.efConstruction = std::stoi(value);}),

	Param("--efSearch", "-eS", "count of tracked nearest nodes during search",
		[](Arguments &args, std::string &value) {args.indexSettings.efSearch = std::stoi(value);}),

	Param("--mL", "prefactor for random level generation",
		[](Arguments &args, std::string &value) {args.indexSettings.mL = std::stod(value);}),

	Param("--keepPrunedConnections", "-k", "if to keep constant number of nodes neighbours",
		[](Arguments &args, std::string&) {args.indexSettings.keepPrunedConnections = true;}),

	Param("--data", "path to file with objects for index",
		[](Arguments &args, std::string &value) {args.dataPath = value;}),

	Param("--dump", "path to file with index dump",
		[](Arguments &args, std::string &value) {args.dumpPath = value;}),

	Param("--base", "count of object, that will be inserted sequentially",
		[](Arguments &args, std::string &value) {args.baseSize = std::stoi(value);}),

	Param("--port", "port, that web-server listen to",
		[](Arguments &args, std::string &value) {args.port = std::stoi(value);}),
};

void printHelp() {
	for (const Param &param : params) {
		param.print();
	}
}

Arguments parseArguments(int argc, char **argv) {
	Arguments args;

	for (int i = 0; i < argc; ++i) {
		std::istringstream argStream(argv[i]);

		std::string paramName;
		std::string value;

		getline(argStream, paramName, '=');
		getline(argStream, value);

		for (const Param &param : params) {
			if (param.check(paramName)) {
				param.apply(args, value);
				break;
			}
		}
	}

	return args;
}
