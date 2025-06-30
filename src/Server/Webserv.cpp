/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:53 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/25 11:29:23 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include <iostream>

Webserv ::Webserv(const std::string &filepath)
{
    _config.readFile(filepath);
}

void Webserv::run()
{
    
}