#include "../Sink/ConsoleSink.hpp"
#include "../Sink/FileSink.hpp"
#include "../LogBuilder.hpp"
#include "../Logger.hpp"
#include "../Form/JsonForm.hpp"
#include "../Form/ElfForm.hpp"
#include <iostream>
#include <stdexcept>

int main() {
	try {
		Logger &logger = Logger::getInstance();
		logger.setLogLevel(INFO);
		logger.addSink(new ConsoleSink(new ElfForm()));
		logger.addSink(new FileSink("./server.log", new JsonForm()));

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
