#include "CgiEndEvent.hpp"

#include "../../../Http/Builder/ResponseBuilder.hpp"
#include "../../../Lib/Logger/Log.hpp"
#include "../../Server.hpp"

#include <cstring>
#include <signal.h>

CgiEndEvent::CgiEndEvent(Client *client, CgiWorker &worker)
	: AEvent(client, client->getContext(), client->getHttpConnection(),
			 EPOLLIN),
	  ACgiEvent(worker) {}

CgiEndEvent::~CgiEndEvent() {}

void CgiEndEvent::handle() { CgiHandle(); }

void CgiEndEvent::process() {

	// CGIの出力を読み切る
	char buffer[16];
	ssize_t bytesRead;
	while ((bytesRead = ::read(_worker.getCompletionFdOut(), buffer,
							   sizeof(buffer))) > 0) {
	}
	if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
		LOG(ERROR) << "Failed to read from CGI completion fd: "
				   << std::strerror(errno);
	}

	if (_worker.isCompletionNotified()) {
		return;
	}
	_worker.setCompletionNotified();

	const int clientFd = _client.getFd();
	LOG(DEBUG) << "" << attr("clientFd", clientFd) << attr("_fd", _fd);

	// レスポンスを送信する
	HttpResponse cgiRes(_context.conf);
	// レスポンスを作成
	_worker.getManager()->isCgiComplete(clientFd, cgiRes);
	const std::string responseStr = ResponseBuilder::build(cgiRes);
	if (!responseStr.empty()) {
		_client.getSocket().setSendBuffer(_client.getSocket().getSendBuffer() +
										  responseStr);
	}
	if (_client.getSocket().getSendBuffer().empty() == false) {
		_client.getServer().getSocketsManager().modifySocket(clientFd,
															 EPOLLOUT);
	}
	_client.getServer().getSocketsManager().unregisterSocket(_fd);

	// handleで終了処理みたいなことをやっている都合、ここで閉じる
	::close(_fd);
	EventManager &eventManager = _client.getEventManager();
	eventManager.forgetFd(_fd);
}
