/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 04:10:11 by teando            #+#    #+#             */
/*   Updated: 2025/06/25 11:22:14 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Config.hpp"

class Webserv
{
    private:
        Config _config;
    public:
        Webserv(const std::string &filepath);
        void run();
};
