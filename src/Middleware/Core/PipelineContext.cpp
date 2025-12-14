#include "PipelineContext.hpp"
#include "../../Server/Client.hpp"

PipelineContext::PipelineContext(const Config &c, Client &client,
								 CgiManager &serverCgiManager)
	: conf(c), req(c), res(c), session(NULL), recvBuffer(""), sendBuffer(""),
	  ownerClient(client), cgiManager(serverCgiManager), isCgi(false),
	  parser(conf) {}

PipelineContext::~PipelineContext() {}

void PipelineContext::reset(const Config &c) {
	req.clear(c);
	res.clear(c);
	recvBuffer.clear();
	sendBuffer.clear();
	isCgi = false;
	parser.reset();
}
