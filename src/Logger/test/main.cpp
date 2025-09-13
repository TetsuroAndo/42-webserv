#include "../Logger.hpp"
#include <iostream>
#include <stdexcept>

int main() {
	try {
		Logger &logger = Logger::getInstance();
		logger.setLogDir("./logs");

		logger.setSinkConsole(JSON, DEBUG);
		logger.setSinkFile("server.log", JSON, DEBUG);
		logger.setSinkFile("WarningOnly.log", ELF, WARNING, EXACT);
		logger.setSinkFile("test_log", "dir_test.log", ELF, DEBUG, EXACT);
		logger.setSinkFile("test_log", "MaxTest.log", ELF, DEBUG, EXACT, 100, 3);

		LOG(DEBUG) << "This is a debug message. It should not appear.";
		LOG(INFO) << "Server is starting...";
		
		std::string clientIp = "127.0.0.1";
		int clientFd = 5;

		// std::stringstreamを使わずに直接数値を渡せる
		LOG(INFO) << "Accepted new connection"
				  << attr("client_ip", clientIp)
				  << attr("fd", clientFd);
		
		LOG(WARNING) << "Configuration file has a deprecated option.";

		LOG(ERROR) << "Failed to process request for resource: /test.html"
				   << attr("status_code", 404) // 数値を直接渡す
				   << attr("reason", "File not found");

	} catch (const std::exception &e) {
		std::cerr << "A critical error occurred: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
