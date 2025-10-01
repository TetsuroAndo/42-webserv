#include "Server/Server.hpp"
#include "Lib/Message/Help.hpp"
#include "Lib/Logger/ErrorLog/LogBuilder.hpp"
#include <iostream>

#define VERSION "0.9"

int main(const int argc, char **argv) {
	try {
		switch (argc) {
			case 1: {
				Config config;
				Server server(config);
				LOG(INFO) << "Webserv version " << VERSION << " starting...";
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
					Config config(argv[1]);
					Server server(config);
					LOG(INFO) << "Webserv version " << VERSION << " starting...";
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
		std::cerr << "[42/Webserv Error] " << e.what() << std::endl;
		return 1;
	}
	LOG(INFO) << "42/Webserv shutting down gracefully.";
	return 0;
}
