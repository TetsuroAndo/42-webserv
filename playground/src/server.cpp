/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:02:41 by teando            #+#    #+#             */
/*   Updated: 2025/01/13 00:03:09 by teando           ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "server.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>     // close()
#include <fcntl.h>      // fcntl()
#include <sys/socket.h> // socket(), bind(), listen(), recv(), send()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr()
#include <poll.h>

// ユーティリティマクロ
#define BUFFER_SIZE 1024
#define BACKLOG 128

Server::Server() : _listenFd(-1) {}

Server::~Server()
{
    if (_listenFd >= 0)
    {
        close(_listenFd);
    }
}

bool Server::init(const std::string &host, int port)
{
    // ソケット生成
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        std::cerr << "Error: socket() failed\n";
        return false;
    }

    // ソケットをノンブロッキングモードに設定
    int flags = fcntl(_listenFd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    if (fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "Error: fcntl() failed\n";
        return false;
    }

    // ソケット再利用設定 (TIME_WAIT回避)
    int enable = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        std::cerr << "Error: setsockopt() failed\n";
        return false;
    }

    // bind()
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str()); // host = "0.0.0.0" など

    if (bind(_listenFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Error: bind() failed\n";
        return false;
    }

    // listen()
    if (listen(_listenFd, BACKLOG) < 0)
    {
        std::cerr << "Error: listen() failed\n";
        return false;
    }

    // pollで監視するリストに追加
    struct pollfd pfd;
    pfd.fd = _listenFd;
    pfd.events = POLLIN; // 読み込み可能（接続待ち受け）
    pfd.revents = 0;
    _fds.push_back(pfd);

    std::cout << "Server listening on " << host << ":" << port << std::endl;
    return true;
}

void Server::run()
{
    std::cout << "Server is running...\n";

    while (true)
    {
        // poll() 実行
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret < 0)
        {
            std::cerr << "Error: poll() failed\n";
            break;
        }

        // 各ソケットごとに処理
        for (size_t i = 0; i < _fds.size(); ++i)
        {
            // イベントがないならスキップ
            if (_fds[i].revents == 0)
                continue;

            // リッスンソケットにイベント(POLLIN)があった場合、新規接続をacceptする
            if (_fds[i].fd == _listenFd && (_fds[i].revents & POLLIN))
            {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int clientFd = accept(_listenFd, (struct sockaddr *)&client_addr, &addrlen);
                if (clientFd >= 0)
                {
                    // ノンブロッキング設定
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    if (flags < 0)
                        flags = 0;
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

                    // pollに追加
                    struct pollfd cfd;
                    cfd.fd = clientFd;
                    cfd.events = POLLIN | POLLOUT;
                    cfd.revents = 0;
                    _fds.push_back(cfd);

                    std::cout << "Accepted new connection (fd = " << clientFd << ")\n";
                }
            }
            // クライアントソケットから読み込み
            else if (_fds[i].revents & POLLIN)
            {
                char buffer[BUFFER_SIZE];
                std::memset(buffer, 0, BUFFER_SIZE);
                int n = recv(_fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                if (n <= 0)
                {
                    // 切断またはエラー
                    std::cout << "Connection closed or error. fd = " << _fds[i].fd << std::endl;
                    close(_fds[i].fd);
                    _fds.erase(_fds.begin() + i);
                    // eraseしたのでイテレータが無効になるため調整
                    --i;
                }
                else
                {
                    // 受信データを簡易的に解析 (本来はHTTPプロトコル解析が必要)
                    std::string request(buffer);

                    // 最小限のGETチェック
                    bool isGetRequest = false;
                    if (request.find("GET ") == 0)
                    {
                        isGetRequest = true;
                    }

                    // レスポンス生成
                    // HTTPレスポンスヘッダを最低限だけつける
                    std::string response;
                    if (isGetRequest)
                    {
                        response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 25\r\n"
                                   "\r\n"
                                   "Hello from minimal server!\n";
                    }
                    else
                    {
                        response = "HTTP/1.1 405 Method Not Allowed\r\n"
                                   "Content-Length: 0\r\n\r\n";
                    }

                    send(_fds[i].fd, response.c_str(), response.size(), 0);

                    // 1リクエストで切断する（超簡単化）
                    close(_fds[i].fd);
                    _fds.erase(_fds.begin() + i);
                    --i;
                }
            }
        }
    }
}
