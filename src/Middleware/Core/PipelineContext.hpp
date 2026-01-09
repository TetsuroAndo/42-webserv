#pragma once

#include "../../Cgi/CgiManager.hpp"
#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include "../../Http/Parser/RequestParser.hpp"
#include "../../Session/Session.hpp"
#include <cstddef>

class CgiManager;
class Client;

/**
 * @brief ミドルウェア間で引き回す情報をまとめた構造体
 */
struct PipelineContext {
	const Config *conf;
	HttpRequest req;
	HttpResponse res;
	Session *session;
	std::string recvBuffer;
	std::string sendBuffer;
	Client &ownerClient;
	CgiManager &cgiManager;
	bool isCgi;
	RequestParser parser;

	PipelineContext(const Config &c, size_t maxHeaderBytes, Client &client,
					CgiManager &serverCgiManager);
	~PipelineContext();
	void setConfig(const Config &c);
	void setError(int code);
	void reset(const Config &c);
};
