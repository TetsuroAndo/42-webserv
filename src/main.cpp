#include "Server/Server.hpp"
#include <iostream>

int main(const int argc, char **argv) {
    try {
    	Config config;
    	if (argc != 1) {
    		config = Config(argv[1]);
    	}
        Server server(config);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
