#pragma once

#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include "../../Http/Parser/RequestParser.hpp"
#include "../../Session/Session.hpp"

class Client;

/**
 * @brief ミドルウェア間で引き回す情報をまとめた構造体
 * @note HttpRequest, HttpResponse, Config, Session への参照を保持する
 */
struct PipelineContext {
	const Config	&conf;
	HttpRequest		*req;
	HttpResponse	*res;
	Session			*session;
	std::string		recvBuffer;
	std::string		sendBuffer;
	RequestParser	parser;
	Client			&ownerClient;

	PipelineContext(const Config &c, Client &client);
	~PipelineContext();
	void reset();
};
