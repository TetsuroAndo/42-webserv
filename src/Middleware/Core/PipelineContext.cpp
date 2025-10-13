#include "PipelineContext.hpp"

PipelineContext::PipelineContext(HttpRequest *r, HttpResponse *s,
								 const Config &c, Client &client)
	: req(r), res(s), conf(c), session(NULL), recvBuffer(""), sendBuffer(""),
	  ownerClient(client) {}

PipelineContext::~PipelineContext() {
	delete req;
	delete res;
}

void PipelineContext::reset() {
	delete req;
	delete res;
	req = new HttpRequest(conf);
	res = new HttpResponse(conf);
	recvBuffer.clear();
	sendBuffer.clear();
	parser.reset();
	if (session) {
		delete session;
		session = NULL;
	}
}
