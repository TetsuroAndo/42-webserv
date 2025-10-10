#include "Client.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Lib/Logger/Log.hpp"
#include "Server.hpp"
#include <arpa/inet.h>
#include <sstream>

Client::Client(const int fd, const sockaddr_in &addr, const Config &config, Server *server)
	: _fd(fd), _server(server) {
	std::stringstream ipStream;
	uint32_t ip_addr = ntohl(addr.sin_addr.s_addr);
	ipStream << ((ip_addr >> 24) & 0xFF) << "."
			 << ((ip_addr >> 16) & 0xFF) << "."
			 << ((ip_addr >> 8) & 0xFF) << "."
			 << (ip_addr & 0xFF);
	_ip = ipStream.str();
	_port = ntohs(addr.sin_port);

	_socket = new Socket(fd, addr);
	HttpRequest *req = new HttpRequest();
	HttpResponse *res = new HttpResponse(SERVER_NAME);
	_context = new PipelineContext(req, res, config);
}

Client::~Client() {
	delete _socket;
	delete _context; // PipelineContext destructor handles deleting req and res
}

int Client::getFd() const { return _fd; }

Socket *Client::getSocket() const { return _socket; }

PipelineContext *Client::getContext() const { return _context; }

const std::string &Client::getIp() const { return _ip; }

int Client::getPort() const { return _port; }

void Client::onTimeout() {
	if (_server) {
		LOG(INFO) << "Client timed out for fd: " << _fd;
		_server->closeConnection(this->getFd());
	}
}
