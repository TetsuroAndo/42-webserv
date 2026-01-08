#include "PipelineContext.hpp"
#include "../../Server/Client/Client.hpp"

// clang-format off
PipelineContext::PipelineContext(const Config &c, Client &client,
								 CgiManager &serverCgiManager)
	: conf(&c),
	  req(c),
	  res(c),
	  session(NULL),
	  recvBuffer(""),
	  sendBuffer(""),
	  ownerClient(client),
	  cgiManager(serverCgiManager),
	  isCgi(false),
	  parser(c.getMaxRequestHeaderSize()) {}
// clang-format on

PipelineContext::~PipelineContext() {}

void PipelineContext::setConfig(const Config &c) {
	conf = &c;
	req.setMaxBodySize(c.getMaxRequestBodySize());
	res.setServerName(c.getAppInfo().httpServerName);
	res.setVersion(c.getAppInfo().httpProtocolVersion);
}

void PipelineContext::setError(int code) {
	res.setStatusCode(code);
	parser.setErrorCode(code);
}

void PipelineContext::reset(const Config &c) {
	conf = &c;
	req.clear(c);
	res.clear(c);
	res.setServerName(c.getAppInfo().httpServerName);
	res.setVersion(c.getAppInfo().httpProtocolVersion);
	recvBuffer.clear();
	sendBuffer.clear();
	isCgi = false;
	parser.reset();
}
