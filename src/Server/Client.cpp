#include "Client.hpp"
#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "../Middleware/Core/PipelineContext.hpp"
#include <netinet/in.h>
#include <sstream>

namespace {
std::stringstream ipToString(uint32_t ip_addr) {
	std::stringstream ss;
	ss << ((ip_addr >> 24) & 0xFF) << "." << ((ip_addr >> 16) & 0xFF) << "."
	   << ((ip_addr >> 8) & 0xFF) << "." << (ip_addr & 0xFF);
	return ss;
}
} // namespace

Client::Client(const int fd, const sockaddr_in &addr, const int listenPort,
			   CgiManager &cgiManager, const Config &config)
	: _fd(fd), _listenPort(listenPort) {
	std::stringstream ipStream;
	const uint32_t ip_addr = ntohl(addr.sin_addr.s_addr);
	_ip = ipToString(ip_addr).str();
	_port = ntohs(addr.sin_port);

	_socket = new Socket(fd, addr);
	_context = new PipelineContext(config, *this, cgiManager);
}

Client::~Client() {
	delete _socket;
	delete _context;
}

int Client::getFd() const { return _fd; }

Socket *Client::getSocket() const { return _socket; }

PipelineContext *Client::getContext() const { return _context; }

const std::string &Client::getIp() const { return _ip; }

int Client::getPort() const { return _port; }

int Client::getListenPort() const { return _listenPort; }
