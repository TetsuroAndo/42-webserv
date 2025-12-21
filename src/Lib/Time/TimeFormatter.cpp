#include "TimeFormatter.hpp"

namespace TimeFormatter {
void getDate(std::string &data, const tm &timeinfo) {
	char dateStr[11];
	if (std::strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo) == 0) {
		data = "0000-00-00";
	} else {
		data = dateStr;
	}
}

void getTime(std::string &data, const tm &timeinfo) {
	char timeStr[9];
	if (std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo) == 0) {
		data = "00:00:00";
	} else {
		data = timeStr;
	}
}

void getHeaderTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	if (std::strftime(timeStr, sizeof(timeStr), "%a, %d %b %Y %H:%M:%S GMT",
				 &timeinfo) == 0) {
		data = "Thu, 01 Jan 1970 00:00:00 GMT";
	} else {
		data = timeStr;
	}
}

void getIsoTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	if (std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", &timeinfo) ==
		0) {
		data = "1970-01-01T00:00:00Z";
	} else {
		data = timeStr;
	}
}

void getLocalTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	if (std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", &timeinfo) ==
		0) {
		data = "1970-01-01T00:00:00";
	} else {
		data = timeStr;
	}
}

void getUtcTimestamp(std::string &data, const tm &timeinfo) {
	char timeStr[64];
	if (std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%SZ", &timeinfo) ==
		0) {
		data = "1970-01-01 00:00:00Z";
	} else {
		data = timeStr;
	}
}
} // namespace TimeFormatter
