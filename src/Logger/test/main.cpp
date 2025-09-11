#include "../Logger.hpp"
#include <iostream>
#include <stdexcept>

int main() {
	try {
		Logger::setLogDir("./logs");
		Logger &logger = Logger::getInstance();
		logger.setSinkConsole(JSON, WARNING);
		logger.setSinkFile("server.log", JSON, INFO);
		logger.setSinkFile("WarningOnly.log", ELF, WARNING, EXACT);
		logger.setSinkFile("test_log", "dir_test.log", ELF, DEBUG, EXACT);

		LOG(DEBUG) << "This is a debug message. It should not appear.";
		LOG(INFO) << "Server is starting...";
		std::string clientIp = "127.0.0.1";
		int clientFd = 5;

		// Convert clientFd to string to use it in the log
		std::stringstream ss;
		ss << clientFd;

		LOG(INFO) << "Accepted new connection"
				  << addAttribute("client_ip", clientIp)
				  << addAttribute("fd", ss.str()); // Use the variable here

		LOG(WARNING) << "Configuration file has a deprecated option.";
		LOG(ERROR) << "Failed to process request for resource: /test.html"
				   << addAttribute("status_code", "404")
				   << addAttribute("reason", "File not found");
	} catch (const std::exception &e) {
		std::cerr << "A critical error occurred: " << e.what() << std::endl;
		Logger::cleanup();
		return 1;
	}
	Logger::cleanup();
	return 0;
}
