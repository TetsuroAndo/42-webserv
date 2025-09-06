#pragma once

#include <string>
#include <vector>
#include <poll.h>

class Server
{
private:
    int _listenFd;
    std::vector<struct pollfd> _fds;

public:
    Server();
    ~Server();
    
    bool init(const std::string& host, int port);
    void run();
};