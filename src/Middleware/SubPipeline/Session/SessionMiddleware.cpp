#include "SessionMiddleware.hpp"

#include "../../../Lib/Logger/Log.hpp"
#include "../../../Lib/StringOps/StringOps.hpp"
#include "../../../Lib/Time/TimeCache.hpp"
#include "../../../Session/Session.hpp"
#include "../../../Session/SessionManager.hpp"
#include "../../Core/MiddlewareProcessor.hpp"
#include "../../Core/PipelineContext.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>

namespace {

std::string unescapeCookieValue(const std::string &in) {
	std::string out;
	out.reserve(in.size());
	bool esc = false;
	for (size_t i = 0; i < in.size(); ++i) {
		char c = in[i];
		if (!esc && c == '\\') {
			esc = true;
			continue;
		}
		out += c;
		esc = false;
	}
	if (esc) {
		out += '\\';
	}
	return out;
}

void extractKeyValue(std::map< std::string, std::string > &result,
					 std::string &current) {
	if (!current.empty()) {
		const size_t pos = current.find('=');
		if (pos == std::string::npos) {
			result[current] = "";
		} else {
			std::string key = current.substr(0, pos);
			std::string value = current.substr(pos + 1);
			StringOps::trim(key);
			StringOps::trim(value);
			const bool hadOuterQuotes =
				(value.size() >= 2 && *value.begin() == '"' &&
				 *(value.end() - 1) == '"');
			if (hadOuterQuotes) {
				value = value.substr(1, value.size() - 2);
			}
			value = unescapeCookieValue(value);
			result[key] = value;
		}
	}
	current.clear();
}

std::map< std::string, std::string >
parseCookieField(const std::string &cookie) {
	std::map< std::string, std::string > result;

	std::string current;
	bool inQuotes = false;
	bool escaped = false;

	for (size_t i = 0; i < cookie.size(); ++i) {
		const char c = cookie[i];
		if (inQuotes && !escaped && c == '\\') {
			escaped = true;
			continue;
		}

		if (c == '"' && !escaped) {
			inQuotes = !inQuotes;
			current += c;
		} else if (c == ';' && !inQuotes && !escaped) {
			StringOps::trim(current);
			extractKeyValue(result, current);
		} else {
			current += c;
		}
		if (escaped) {
			escaped = false;
		}
	}
	if (escaped) {
		current += '\\';
	}
	StringOps::trim(current);
	extractKeyValue(result, current);
	return result;
}
} // namespace

#ifndef UNIT_TEST

SessionMiddleware::SessionMiddleware() {}

SessionMiddleware::~SessionMiddleware() {}

void SessionMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	SessionManager &manager = SessionManager::getInstance();
	std::string token;
	bool isNewSession = false;

	if (ctx.req.getHeader("Cookie").empty() == false) {
		std::map< std::string, std::string > reqCookie =
			parseCookieField(ctx.req.getHeader("Cookie"));
		if (reqCookie.count("sessionId") > 0) {
			token = reqCookie["sessionId"];
		}
	}
	Session *currentSession = NULL;
	if (token.empty() == false) {
		currentSession = manager.getSession(token);
		if (currentSession != NULL) {
			LOG(INFO) << "Session ID: " << currentSession->getId();
		}
	}
	if (NULL == currentSession) {
		currentSession = manager.createSession();
		LOG(INFO) << "Created session: " << currentSession->getId();
		isNewSession = true;
	}
	ctx.session = currentSession;

	if (isNewSession) {
		std::string response = "sessionId=\"";
		response.append(currentSession->getId());
		response.append("\"; Path=/; HttpOnly");
		ctx.res.appendHeader("Set-Cookie", response);
	}

	if (proc) {
		proc->next(ctx);
	}
}

#endif // UNIT_TEST
