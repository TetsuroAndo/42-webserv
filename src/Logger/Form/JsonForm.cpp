#include "JsonForm.hpp"
#include <sstream>
#include <ctime>

/**
 * @brief JSONエスケープ処理
 * @param str エスケープ対象の文字列
 * @return エスケープ後の文字列
 */
std::string JsonForm::escapeJson(const std::string& str) const {
	std::string escaped = str;
	size_t pos = 0;
	while ((pos = escaped.find("\"", pos)) != std::string::npos) {
		escaped.replace(pos, 1, "\\\"");
		pos += 2;
	}
	return escaped;
}

/**
 * @brief JSON形式でログメッセージをフォーマット
 * @param msg ログメッセージ
 * @return フォーマットされたログメッセージ
 */
std::string JsonForm::format(const LogMessage& msg) {
	std::stringstream ss;
	char timeStr[20];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", localtime(&msg.timestamp));

	ss << "{";
	ss << "\"timestamp\":\"" << timeStr << "\",";
	ss << "\"level\":\"" << LogForm::levelToString(msg.level) << "\",";
	ss << "\"message\":\"" << escapeJson(msg.message) << "\",";
	ss << "\"source\":\"" << msg.file << ":" << msg.line << "\"";

	if (!msg.attributes.empty()) {
		ss << ",\"attributes\":{";
		for (std::map<std::string, std::string>::const_iterator it = msg.attributes.begin();
			 it != msg.attributes.end();) {
			ss << "\"" << it->first << "\":\"" << escapeJson(it->second) << "\"";
			if (++it != msg.attributes.end()) {
				ss << ",";
			}
		}
		ss << "}";
	}
	ss << "}";
	return ss.str();
}
