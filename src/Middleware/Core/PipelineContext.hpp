#pragma once

#include "../../Config/Config.hpp"
#include "../../Http/Core/HttpRequest.hpp"
#include "../../Http/Core/HttpResponse.hpp"
#include "../../Session/Session.hpp"

/**
 * @brief ミドルウェア間で引き回す情報をまとめた構造体
 * @note HttpRequest, HttpResponse, Config, Session への参照を保持する
 */
struct PipelineContext {
	HttpRequest *req;
	HttpResponse *res;
	const Config &conf;
	Session *session;
    std::string recvBuffer;
    std::string sendBuffer;  

	PipelineContext(HttpRequest *r, HttpResponse *s, const Config &c);
};
