#include "ElfForm.hpp"
#include "../../../Http/Core/HttpStatus.hpp"
#include "../../StringOps/StringOps.hpp"
#include "../../Time/TimeCache.hpp"
#include <ctime>
#include <sstream>

/**
 * 参考先:
 * https://en.wikipedia.org/wiki/Extended_Log_Format
 * https://www.w3.org/TR/WD-logfile
 * https://docs.aws.amazon.com/ja_jp/athena/latest/ug/querying-iis-logs-w3c-extended-log-file-format.html
 */

namespace {
/**
 * @brief W3C-ELF形式のログメッセージフォーマットで使用するための文字列のサニタイズ
 */
std::string sanitize(const std::string& str) {
	if (str.empty()) {
		return "-";
	}
	if (str.find(' ') == std::string::npos && str.find('"') == std::string::npos) {
		return str;
	}

	std::string result = "\"";
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '"') {
			result += "\"\"";
		} else {
			result += str[i];
		}
	}
	result += '"';
	return result;
}
} // namespace

ElfForm::ElfForm() : _headerWritten(false) {}

void ElfForm::getErrorHeader(std::ostream &out) {
	out << "#Fields: date time level function file:line message attributes\n";
	_headerWritten = true;
}

void ElfForm::getAccessHeader(std::ostream &out) {
	std::string timeStr = TimeCache::getUtcTimestamp();
	out << "#Version: 1.0\n";
	out << "#Date: " << timeStr << "\n";
	out << "#Software: webserv/42\n";
	out << "#Fields: date time c-ip c-port cs-method cs-uri-stem cs-uri-query sc-status sc-bytes cs-version cs(User-Agent) cs(Referer) x-session-id\n";
	_headerWritten = true;
}

void ElfForm::format(const LogMessage &msg, std::ostream &out) {
	if (!_headerWritten) getErrorHeader(out);

	out << msg.localDate << " " << msg.localTime << " ";
	out << LogForm::levelToString(msg.level) << " ";
	out << msg.function << " ";
	out << msg.file << ":" << msg.line << " ";
	out << sanitize(msg.message) << " ";

	if (msg.attributes.empty()) {
		out << "-";
	} else {
		for (std::map<std::string, std::string>::const_iterator it =
				 msg.attributes.begin();
			 it != msg.attributes.end();) {
			out << sanitize(it->first) << "=" << sanitize(it->second);
			if (++it != msg.attributes.end()) {
				out << ";";
			}
		}
	}
}

void ElfForm::formatAccess(const AccessLogContext& ctx, std::ostream& out) {
	if (!ctx.request || !ctx.response) {
		return;
	}
	if (!_headerWritten) getAccessHeader(out);

	out << ctx.utcTimestamp << " ";
	out << (ctx.remote_addr.empty() ? "-" : ctx.remote_addr) << " ";
	out << ctx.client_port << " ";
	out << (ctx.request->getMethod().empty() ? "-" : ctx.request->getMethod()) << " ";
	out << (ctx.request->getPath().empty() ? "-" : ctx.request->getPath()) << " ";

	const std::map<std::string, std::string>& queries = ctx.request->getQueries();
	if (queries.empty()) {
		out << "- ";
	} else {
		std::string queryString;
		for (std::map<std::string, std::string>::const_iterator it = queries.begin(); it != queries.end();) {
			queryString += it->first + "=" + it->second;
			if (++it != queries.end()) {
				queryString += "&";
			}
		}
		out << queryString << " ";
	}

	out << ctx.response->getStatusCode() << " ";
	out << ctx.response->getBody().length() << " ";
	out << (ctx.request->getVersion().empty() ? "-" : ctx.request->getVersion()) << " ";

	out << sanitize(ctx.request->getHeader("User-Agent")) << " ";
	out << sanitize(ctx.request->getHeader("Referer")) << " ";
	out << (ctx.session_id.empty() ? "-" : sanitize(ctx.session_id));
}
