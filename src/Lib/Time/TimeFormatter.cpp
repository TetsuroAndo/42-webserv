#include "TimeFormatter.hpp"

namespace TimeFormatter{
void getDate(std::string &data, const tm &timeinfo) {
	char dateStr[11];
	std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
	data = dateStr;
}

void getTime(std::string &data, const tm &timeinfo) {
	char timeStr[9];
	std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
	data = timeStr;
}

void getHeaderTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
	data = timeStr;
}

void getIsoTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
	data = timeStr;
}

void getLocalTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", &timeinfo);
	data = timeStr;
}

void getUtcTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%SZ", &timeinfo);
	data = timeStr;
}
} // namespace TimeFormatter
