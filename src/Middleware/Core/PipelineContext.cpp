#include "PipelineContext.hpp"
#include "../../Server/Client.hpp"

PipelineContext::PipelineContext(HttpRequest *r, HttpResponse *s,
								 const Config &c, Client &client)
	: conf(c), req(r), res(s), session(NULL), recvBuffer(""), sendBuffer(""),
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
	// sessionはresetでdeleteするべきか要検討
	if (session) {
		delete session;
		session = NULL;
	}
}