/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:05:30 by teando            #+#    #+#             */
/*   Updated: 2025/01/13 00:05:31 by teando           ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <poll.h>

// 簡易サーバークラスの宣言
class Server
{
public:
    Server();
    ~Server();

    // 初期化（ソケット生成・bind・listenなど）
    bool init(const std::string &host, int port);

    // メインループ
    void run();

private:
    int _listenFd;                   // リッスンソケット
    std::vector<struct pollfd> _fds; // poll() で監視するソケット群
};

#endif // SERVER_HPP
