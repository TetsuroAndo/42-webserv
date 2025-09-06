/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 00:04:03 by teando            #+#    #+#             */
/*   Updated: 2025/06/30 11:11:56 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include <iostream>
#include <string>

int main(void)
{
    std::string host = "0.0.0.0";
    int port = 8080;

    // 引数で設定ファイルを受け取る想定だが、ここでは超ミニマルに直接指定
    // 実際には default.conf などからポートやサーバ名などを読み込む
    
    Server server;
    if (!server.init(host, port))
    {
        std::cerr << "Failed to initialize server\n";
        return 1;
    }

    server.run();
    return 0;
}
