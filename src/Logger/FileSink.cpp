#include "FileSink.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

FileSink::FileSink(const std::string &filename)
	: _file(filename.c_str(), std::ios::out | std::ios::app)
{
	if (!_file.is_open()) {
		throw std::runtime_error("Logger: Failed to open log file: " + filename);
	}
}

FileSink::~FileSink() {
	if (_file.is_open()) {
		_file.close();
	}
}

void FileSink::write(const std::string &formattedMessage) {
	if (_file.is_open()) {
		_file << formattedMessage << std::endl;
	}
}
