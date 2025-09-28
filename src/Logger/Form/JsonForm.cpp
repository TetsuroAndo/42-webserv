#include "JsonForm.hpp"
#include <ctime>
#include <sstream>

/**
 * @brief JSONエスケープ処理
 * @param str エスケープ対象の文字列
 * @return エスケープ後の文字列
 */
std::string JsonForm::escapeJson(const std::string &str) const {
	std::stringstream ss;
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		switch (*it) {
		case '"':
			ss << "\\\"";
			break;
		case '\\':
			ss << "\\\\";
			break;
		case '\b':
			ss << "\\b";
			break;
		case '\f':
			ss << "\\f";
			break;
		case '\n':
			ss << "\\n";
			break;
		case '\r':
			ss << "\\r";
			break;
		case '\t':
			ss << "\\t";
			break;
		default:
			ss << *it;
			break;
		}
	}
	return ss.str();
}

/**
 * @brief JSON形式でログメッセージをフォーマット
 * @param msg ログメッセージ
 * @param out 出力ストリーム
 */
void JsonForm::format(const LogMessage &msg, std::ostream &out) {
	char timeStr[20];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S",
			 localtime(&msg.timestamp));

	out << "{";
	out << "\"timestamp\":\"" << timeStr << "\",";
	out << "\"level\":\"" << LogForm::levelToString(msg.level) << "\",";
	out << "\"message\":\"" << escapeJson(msg.message) << "\",";
	out << "\"source\":\"" << msg.file << ":" << msg.line << "\"";
	out << ",\"function\":\"" << msg.function << "\"";

	if (!msg.attributes.empty()) {
		out << ",\"attributes\":{";
		for (std::map<std::string, std::string>::const_iterator it =
				 msg.attributes.begin();
			 it != msg.attributes.end();) {
			out << "\"" << it->first << "\":\"" << escapeJson(it->second)
				<< "\"";
			if (++it != msg.attributes.end()) {
				out << ",";
			}
		}
		out << "}";
	}
	out << "}";
}
