#include "Server/Server.hpp"
#include <iostream>

int main(const int argc, char **argv) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
            return 1;
        }
        Config config(argv[1]);
        Server server(config);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
