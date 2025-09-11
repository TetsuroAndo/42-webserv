#include "ElfForm.hpp"
#include <sstream>
#include <ctime>

/**
 * 参考先:
 * https://en.wikipedia.org/wiki/Extended_Log_Format
 * https://www.w3.org/TR/WD-logfile
 * https://docs.aws.amazon.com/ja_jp/athena/latest/ug/querying-iis-logs-w3c-extended-log-file-format.html
 */

ElfForm::ElfForm() : _headerWritten(false) {}

std::string ElfForm::levelToString(LogLevel level) const {
	switch (level) {
		case DEBUG: return "DEBUG";
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case FATAL: return "FATAL";
		default: return "UNKNOWN";
	}
}

// スペースや特殊文字を '-' に置換
std::string ElfForm::sanitize(const std::string& str) const {
	std::string sanitized = str;
	for (size_t i = 0; i < sanitized.length(); ++i) {
		if (sanitized[i] == ' ' || sanitized[i] == '\t' || sanitized[i] == '\n' || sanitized[i] == '\r') {
			sanitized[i] = '-';
		}
	}
	return sanitized;
}

std::string ElfForm::getHeader() {
	return "#Fields: date time level file:line message attributes";
}

std::string ElfForm::format(const LogMessage& msg) {
	std::stringstream ss;

	if (!_headerWritten) {
		ss << getHeader() << std::endl;
		_headerWritten = true;
	}

	char dateStr[11];
	char timeStr[9];
	strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&msg.timestamp));
	strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&msg.timestamp));

	// date time
	ss << dateStr << " " << timeStr << " ";
	// level
	ss << levelToString(msg.level) << " ";
	// file:line
	ss << msg.file << ":" << msg.line << " ";
	// message
	ss << sanitize(msg.message) << " ";

	// attributes (key=value;key2=value2 形式)
	if (msg.attributes.empty()) {
		ss << "-";
	} else {
		for (std::map<std::string, std::string>::const_iterator it = msg.attributes.begin();
			 it != msg.attributes.end();) {
			ss << sanitize(it->first) << "=" << sanitize(it->second);
			if (++it != msg.attributes.end()) {
				ss << ";";
			}
		}
	}
	return ss.str();
}
