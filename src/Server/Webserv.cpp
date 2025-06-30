/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:53 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 15:17:42 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include <iostream>

Webserv ::Webserv(const std::string &filePath) {
	_serverconfig.readFile(filePath);
}

Webserv::~Webserv() {}
// void Webserv::run(){}