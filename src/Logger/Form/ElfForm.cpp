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
	return "#Fields: date time level function file:line message attributes";
}

void ElfForm::format(const LogMessage& msg, std::ostream& out) {
	if (!_headerWritten) {
		out << getHeader() << "\n";
		_headerWritten = true;
	}
	char dateStr[11];
	char timeStr[9];
	strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", localtime(&msg.timestamp));
	strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localtime(&msg.timestamp));
	
	out << dateStr << " " << timeStr << " ";
	out << LogForm::levelToString(msg.level) << " ";
	out << msg.function << " ";
	out << msg.file << ":" << msg.line << " ";
	out << sanitize(msg.message) << " ";
	
	if (msg.attributes.empty()) {
		out << "-";
	} else {
		for (std::map<std::string, std::string>::const_iterator it = msg.attributes.begin();
			 it != msg.attributes.end();) {
			out << sanitize(it->first) << "=" << sanitize(it->second);
			if (++it != msg.attributes.end()) {
				out << ";";
			}
		}
	}
}
