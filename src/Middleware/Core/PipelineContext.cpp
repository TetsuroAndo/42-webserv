#include "PipelineContext.hpp"
#include "../../Server/Client.hpp"

PipelineContext::PipelineContext(const Config &c, Client &client)
	: conf(c), req(new HttpRequest(c)), res(new HttpResponse(c)), session(NULL),
	  recvBuffer(""), sendBuffer(""), ownerClient(client) {}

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
	// TODO: Consider whether session should be deleted on reset
	if (session) {
		delete session;
		session = NULL;
	}
}
