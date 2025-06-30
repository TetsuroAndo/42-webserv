#include "server.hpp"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>     // close()
#include <fcntl.h>      // fcntl()
#include <sys/socket.h> // socket(), bind(), listen(), recv(), send()
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr()
#include <poll.h>
#include <fstream>      // ファイル読み込み用
#include <sstream>      // HTTPレスポンス構築用

// (Serverクラスの定義やinitメソッドは変更なし)

// ユーティリティマクロ
#define BUFFER_SIZE 1024
#define BACKLOG 128

// コンストラクタ
Server::Server() : _listenFd(-1)
{
}

// デストラクタ
Server::~Server()
{
    if (_listenFd >= 0)
    {
        close(_listenFd);
    }
    
    // すべてのクライアントソケットを閉じる
    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd >= 0 && _fds[i].fd != _listenFd)
        {
            close(_fds[i].fd);
        }
    }
}

// サーバー初期化
bool Server::init(const std::string& host, int port)
{
    // ソケット作成
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
    {
        std::cerr << "Error: socket() failed\n";
        return false;
    }
    

    // SO_REUSEADDR オプションを設定
    int opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Error: setsockopt() failed\n";
        close(_listenFd);
        return false;
    }

    // ノンブロッキングに設定
    int flags = fcntl(_listenFd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK);

    // アドレス設定
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    // バインド
    if (bind(_listenFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error: bind() failed\n";
        close(_listenFd);
        return false;
    }

    // リッスン
    if (listen(_listenFd, BACKLOG) < 0)
    {
        std::cerr << "Error: listen() failed\n";
        close(_listenFd);
        return false;
    }

    // pollfdに追加
    struct pollfd listen_pfd;
    listen_pfd.fd = _listenFd;
    listen_pfd.events = POLLIN;
    listen_pfd.revents = 0;
    _fds.push_back(listen_pfd);

    std::cout << "Server initialized on " << host << ":" << port << std::endl;
    return true;
}


void Server::run()
{
    std::cout << "Server is running...\n";

    while (true)
    {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret < 0)
        {
            std::cerr << "Error: poll() failed\n";
            break;
        }

        for (size_t i = 0; i < _fds.size(); ++i)
        {
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
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    if (flags < 0)
                        flags = 0;
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

                    struct pollfd cfd;
                    cfd.fd = clientFd;
                    cfd.events = POLLIN | POLLOUT; // 読み込みと書き込みの両方を監視
                    cfd.revents = 0;
                    _fds.push_back(cfd);

                    // ここで新しいクライアント接続の状態を管理するデータ構造も必要になります。
                    // 例: std::map<int, ClientConnectionState> _clientStates;
                    // _clientStates[clientFd] = ClientConnectionState();

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
                    // 切断またはエラー (EAGAIN/EWOULDBLOCK以外)
                    std::cout << "Connection closed or error. fd = " << _fds[i].fd << std::endl;
                    close(_fds[i].fd);
                    _fds.erase(_fds.begin() + i);
                    // ここで _clientStates からもエントリを削除
                    // _clientStates.erase(_fds[i].fd);
                    --i;
                }
                else
                {
                    // ここにHTTPリクエストパースロジックを実装
                    // 現在は簡略化

                    std::string received_data(buffer, n);
                    // 本来は部分受信を考慮し、バッファに追記していく
                    // 例: _clientStates[fd].appendToReceiveBuffer(received_data);
                    //     if (_clientStates[fd].isRequestComplete()) { ... }

                    // *** ここから index.html を返すための修正 ***
                    std::string request_uri = "/"; // デフォルトとしてルートパスを仮定
                                                   // 実際のURIはパースで取得

                    std::string file_path = "./html/index.html"; // 仮のパス。本来はLocationConfigから決定

                    std::ifstream ifs(file_path.c_str());
                    std::stringstream ss;
                    std::string response_body;

                    if (ifs.is_open())
                    {
                        ss << ifs.rdbuf();
                        response_body = ss.str();
                        ifs.close();

                        // HTTPレスポンスヘッダ
                        std::string response_header = "HTTP/1.1 200 OK\r\n";
                        response_header += "Content-Type: text/html\r\n";

                        std::stringstream ss_length; // << Content-Length 用のstringstream
                        ss_length << response_body.length(); // response_body.length() をstringstreamに入れる
                        response_header += "Content-Length: " + ss_length.str() + "\r\n"; // 文字列に変換して追加
                        response_header += "\r\n"; // ヘッダの終端

                        std::string full_response = response_header + response_body;

                        // ここでレスポンスを送信バッファに格納し、POLLOUTイベントを待つべき
                        // 例: _clientStates[_fds[i].fd].setResponseToSend(full_response);
                        // 現在はデモのために直接send
                        send(_fds[i].fd, full_response.c_str(), full_response.size(), 0);
                        std::cout << "Sent index.html to fd: " << _fds[i].fd << std::endl;

                        // 簡易化のため1リクエストで切断
                        close(_fds[i].fd);
                        _fds.erase(_fds.begin() + i);
                        --i;
                    }
                    else
                    {
                        // ファイルが見つからない場合のエラー処理
                        std::string not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                                       "Content-Type: text/plain\r\n"
                                                       "Content-Length: 13\r\n"
                                                       "\r\n"
                                                       "404 Not Found\n";
                        send(_fds[i].fd, not_found_response.c_str(), not_found_response.size(), 0);
                        std::cout << "Sent 404 Not Found to fd: " << _fds[i].fd << std::endl;

                        // 簡易化のため1リクエストで切断
                        close(_fds[i].fd);
                        _fds.erase(_fds.begin() + i);
                        --i;
                    }
                    // *** 修正ここまで ***
                }
            }
            // POLLOUTイベント処理 (送信バッファにデータがある場合のみ送信)
            // このブロックを実装する必要があります。
            // else if (_fds[i].revents & POLLOUT) {
            //     // _clientStates[fd].sendDataIfAvailable();
            //     // 送信完了したら、_fds[i].events から POLLOUT を削除する
            //     // または、接続を閉じる
            // }
        }
    }
}