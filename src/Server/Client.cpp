#include "../Http/Core/HttpRequest.hpp"
#include "../Http/Core/HttpResponse.hpp"
#include "Client.hpp"

Client::Client(const int fd, const sockaddr_in &addr, const Config &config)
    : _fd(fd) {
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
