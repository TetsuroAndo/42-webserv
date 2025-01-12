/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:04:03 by teando            #+#    #+#             */
/*   Updated: 2025/01/13 00:04:05 by teando           ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "server.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::string host = "0.0.0.0";
    int port = 8080;

    // 引数で設定ファイルを受け取る想定だが、ここでは超ミニマルに直接指定
    // 実際には default.conf などからポートやサーバ名などを読み込む
    if (argc == 2)
    {
        // 例: ポート番号だけ読み込むケース
        port = std::atoi(argv[1]);
    }

    Server server;
    if (!server.init(host, port))
    {
        std::cerr << "Failed to initialize server\n";
        return 1;
    }

    server.run();
    return 0;
}
