#include "Server/Server.hpp"
#include "Config/ConfigBuilder.hpp"
#include "Lib/Message/Help.hpp"
#include "Lib/Logger/ErrorLog/LogBuilder.hpp"
#include <iostream>

int main(const int argc, char **argv) {
	try {
		switch (argc) {
			case 1: {
				Server server(ConfigBuilder().build());
				server.run();
				break;
			}
			case 2: {
				std::string arg = argv[1];
				if (arg == "-h" || arg == "--help") {
					printHelp(argv[0]);
				} else if (arg == "-v" || arg == "--version") {
					printVersion(VERSION);
				} else {
					Server server(ConfigBuilder(argv[1]).build());
					server.run();
				}
				break;
			}
			default:
				printUsage(argv[0]);
				return 1;
		}
	} catch (const std::exception &e) {
		LOG(FATAL) << "Server failed to start: " << e.what();
		std::cerr << "[ FATAL ] " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
