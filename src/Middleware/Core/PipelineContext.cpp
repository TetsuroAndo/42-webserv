#include "PipelineContext.hpp"

PipelineContext::PipelineContext(HttpRequest *r, HttpResponse *s,
								 const Config &c)
	: req(r), res(s), conf(c), session(NULL), recvBuffer(""), sendBuffer("") {}

PipelineContext::~PipelineContext() {
	delete req;
	delete res;
}

void PipelineContext::reset() {
	std::string serverName = res->getServerName();
	delete req;
	delete res;
	req = new HttpRequest();
	res = new HttpResponse(serverName);
	recvBuffer.clear();
	sendBuffer.clear();
	parser.reset();
	if (session) {
		delete session;
		session = NULL;
	}
}
