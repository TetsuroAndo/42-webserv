#pragma once

#include <ctime>
#include <string>

namespace TimeFormatter{
	void getDate(std::string &data, const tm &timeinfo);
	void getTime(std::string &data, const tm &timeinfo);
	void getIsoTimestamp(std::string &data, const tm &timeinfo);
	void getLocalTimestamp(std::string &data, const tm &timeinfo);
	void getUtcTimestamp(std::string &data, const tm &timeinfo);
	void getHeaderTimestamp(std::string &data, const tm &timeinfo);
} // namespace TimeFormatter
