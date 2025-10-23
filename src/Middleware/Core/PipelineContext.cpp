#include "PipelineContext.hpp"
#include "../../Server/Client.hpp"

PipelineContext::PipelineContext(const Config &c, Client &client,
								 CgiManager &serverCgiManager)
	: conf(c), req(new HttpRequest(c)), res(new HttpResponse(c)), session(NULL),
	  recvBuffer(""), sendBuffer(""), ownerClient(client),
	  cgiManager(serverCgiManager) {}

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
}
