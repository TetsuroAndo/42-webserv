#include "SessionMiddleware.hpp"

#include "../../../Lib/StringOps/StringOps.hpp"
#include "../../../Lib/Time/TimeCache.hpp"
#include "../../Core/MiddlewareProcessor.hpp"
#include "../../Core/PipelineContext.hpp"
#include "../../../Session/Session.hpp"
#include "../../../Session/SessionManager.hpp"
#include "../../../Lib/Logger/Log.hpp"

#include <string>
#include <map>
#include <sstream>
#include <algorithm>

namespace {

std::map<std::string, std::string> parseCookieField(const std::string &cookie) {
	std::map<std::string, std::string> result;
	std::istringstream stream(cookie);
	std::string pair;

	while (std::getline(stream, pair, ';')) {
		StringOps::trim(pair);

		size_t pos = pair.find('=');
		if (pos == std::string::npos) {
			result[pair] = "";
		} else {
			std::string key = pair.substr(0, pos);
			std::string value = pair.substr(pos + 1);
			StringOps::trim(key);
			StringOps::trim(value);
			result[key] = value;
		}
	}

	return result;
}
}

SessionMiddleware::SessionMiddleware() {}

SessionMiddleware::~SessionMiddleware() {}

void SessionMiddleware::handle(PipelineContext &ctx,
							   MiddlewareProcessor *proc) {
	(void)ctx;
	SessionManager &manager = SessionManager::getInstance();
	std::string token;
	if (ctx.req->getHeader("Cookie").empty() == false) {
		std::map<std::string, std::string> reqCookie = parseCookieField(ctx.req->getHeader("Cookie"));
		if (reqCookie.count("sessionId") > 0) {
			token = reqCookie["sessionId"];
		}
	}
	Session *currentSession = NULL;
	if (token.empty() == false) {
		 currentSession = manager.getSession(token);
		LOG(INFO) << "Session ID :" << ctx.session->getId();
	}
	if (NULL == currentSession) {
		currentSession = manager.createSession();
		LOG(INFO) << "Created session " << currentSession->getId();
	}
	ctx.session = currentSession;

	std::string response = "sessionId=";
	response.append(currentSession->getId());
	response.append("; Path=/; HttpOnly");
	ctx.res->setHeader("Set-Cookie", response);

	std::string response2 = "lastAccessTime=";
	response2.append(TimeCache::getLocalTimestamp());
	ctx.res->setHeader("Set-Cookie", response2);

	if (proc) {
		proc->next(ctx);
	}
}
