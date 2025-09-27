#include "Server/Server.hpp"
#include <iostream>

int main(const int argc, char **argv) {
	try {
		// ******************************************
		// 将来的に置き換え
		//    	Config config;
		//    	if (argc != 1) {
		//    		config = Config(argv[1]);
		//    	}
		//        Server server(config);
		(void)argc;
		(void)argv;
		// ******************************************
		Server server;
		server.run();
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
