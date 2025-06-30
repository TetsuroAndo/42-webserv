/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 04:10:11 by teando            #+#    #+#             */
/*   Updated: 2025/06/30 15:16:05 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ServerConfig.hpp"

class Webserv
{
    private:
        ServerConfig    _serverconfig;
    public:
        Webserv(const std::string &filePath);
        ~Webserv();
        // void run();
};

