/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 04:10:11 by teando            #+#    #+#             */
/*   Updated: 2025/06/30 17:23:37 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "ServerConfig.hpp"
#include <vector>

class Webserv
{
    private:
        std::vector<ServerConfig> _serverconfigs;
    public:
        Webserv(const std::string &filePath);
        ~Webserv();
        void run();
        const std::vector<ServerConfig>& getServerConfigs() const;
        void printConfigs() const;
};

std::ostream &operator<<(std::ostream &os, const ServerConfig &config);