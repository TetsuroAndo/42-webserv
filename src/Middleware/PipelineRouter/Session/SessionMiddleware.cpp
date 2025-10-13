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
			if (value.size() >= 2 && *value.begin() == '"' &&
				*(value.end() - 1) == '"') {
				value = value.substr(1, value.size() - 2);
			}
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

	for (size_t i = 0; i < cookie.size(); ++i) {
		const char c = cookie[i];

		if (c == '"') {
			inQuotes = !inQuotes;
			current += c;
		} else if (c == ';' && !inQuotes) {
			StringOps::trim(current);
			extractKeyValue(result, current);
		} else {
			current += c;
		}
	}
	StringOps::trim(current);
	extractKeyValue(result, current);
	return result;
}
} // namespace

SessionMiddleware::SessionMiddleware() {}

SessionMiddleware::~SessionMiddleware() {}

void SessionMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	(void)ctx;
	SessionManager &manager = SessionManager::getInstance();
	std::string token;
	if (ctx.req->getHeader("Cookie").empty() == false) {
		std::map< std::string, std::string > reqCookie =
			parseCookieField(ctx.req->getHeader("Cookie"));
		if (reqCookie.count("sessionId") > 0) {
			token = reqCookie["sessionId"];
			LOG(DEBUG) << "Received sessionId: " << token;
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
	}
	ctx.session = currentSession;

	{
		std::string response = "sessionId=\"";
		response.append(currentSession->getId());
		response.append("\"; Path=/; HttpOnly");
		ctx.res->setHeader("Set-Cookie", response);
	}
	{
		std::string response = "lastAccessTime=";
		response.append(TimeCache::getLocalTimestamp());
		response.append("; Path=/");
		ctx.res->setHeader("Set-Cookie", response);
	}
	{
		std::string response = "serverName=";
		response.append(ctx.res->getServerName());
		response.append("; Path=/");
		ctx.res->setHeader("Set-Cookie", response);
	}

	if (proc) {
		proc->next(ctx);
	}
}
