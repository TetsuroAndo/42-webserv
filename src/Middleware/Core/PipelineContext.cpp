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
	changes = FdEventChanges();
}

void PipelineContext::addChanges(const FdEventChanges &additionalChanges) {
	changes.fdsToAdd.insert(changes.fdsToAdd.end(),
							additionalChanges.fdsToAdd.begin(),
							additionalChanges.fdsToAdd.end());
	changes.fdsToRemove.insert(changes.fdsToRemove.end(),
							   additionalChanges.fdsToRemove.begin(),
							   additionalChanges.fdsToRemove.end());
	changes.clientFdsToNotify.insert(
		changes.clientFdsToNotify.end(),
		additionalChanges.clientFdsToNotify.begin(),
		additionalChanges.clientFdsToNotify.end());
}
