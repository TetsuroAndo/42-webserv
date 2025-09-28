#include "AccessLogForm.hpp"

#include <sstream>

namespace {
	std::string getAttr(const std::map<std::string, std::string>& attributes, const std::string& key, const std::string& defaultValue = "-") {
		std::map<std::string, std::string>::const_iterator it = attributes.find(key);
		if (it != attributes.end()) {
			return it->second;
		}
		return defaultValue;
	}
}

void AccessLogForm::format(const LogMessage& msg, std::ostream& out) {
	char timeStr[64];
	time_t timestamp = msg.timestamp;
	struct tm *localTime = localtime(&timestamp);
	strftime(timeStr, sizeof(timeStr), "%d/%b/%Y:%H:%M:%S %z", localTime);

	out << getAttr(msg.attributes, "remote_addr") << " "
		<< getAttr(msg.attributes, "remote_user") << " "
		<< "[" << timeStr << "] "
		<< "\"" << getAttr(msg.attributes, "request_line") << "\" "
		<< getAttr(msg.attributes, "status_code") << " "
		<< getAttr(msg.attributes, "body_bytes_sent") << " "
		<< "\"" << getAttr(msg.attributes, "http_referer") << "\" "
		<< "\"" << getAttr(msg.attributes, "http_user_agent") << "\"";
}
