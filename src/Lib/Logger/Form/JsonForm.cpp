#include "JsonForm.hpp"
#include "../../Http/Core/HttpStatus.hpp"
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
			 localtime(&msg.timestamp)); // TODO: キャッシュから呼び出すようにする

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

void JsonForm::formatAccess(const AccessLogContext& ctx, std::ostream& out) {
	if (!ctx.request || !ctx.response) {
		return;
	}

	char timeStr[21];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", gmtime(&ctx.timestamp)); // TODO: キャッシュから呼び出すようにする

	std::string uri = ctx.request->getPath();
	const std::map<std::string, std::string>& queries = ctx.request->getQueries();
	if (!queries.empty()) {
		uri += "?";
		for (std::map<std::string, std::string>::const_iterator it = queries.begin(); it != queries.end();) {
			uri += it->first + "=" + it->second;
			if (++it != queries.end()) {
				uri += "&";
			}
		}
	}

	out << "{";
	out << "\"timestamp\":\"" << timeStr << "\",";
	out << "\"remote_addr\":\"" << escapeJson(ctx.remote_addr) << "\",";
	out << "\"remote_port\":" << ctx.client_port << ",";
	out << "\"method\":\"" << escapeJson(ctx.request->getMethod()) << "\",";
	out << "\"uri\":\"" << escapeJson(uri) << "\",";
	out << "\"version\":\"" << escapeJson(ctx.request->getVersion()) << "\",";
	out << "\"status\":" << ctx.response->getStatusCode() << ",";
	out << "\"bytes_sent\":" << ctx.response->getBody().length() << ",";
	
	const std::string& referer = ctx.request->getHeader("Referer");
	out << "\"referer\":\"" << (referer.empty() ? "-" : escapeJson(referer)) << "\",";

	const std::string& userAgent = ctx.request->getHeader("User-Agent");
	out << "\"user_agent\":\"" << (userAgent.empty() ? "-" : escapeJson(userAgent)) << "\",";

	out << "\"session_id\":\"" << (ctx.session_id.empty() ? "-" : escapeJson(ctx.session_id)) << "\"";
	out << "}";
}
