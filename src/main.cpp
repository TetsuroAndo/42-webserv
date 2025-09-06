/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:43 by hirwatan          #+#    #+#             */
/*   Updated: 2025/09/06 11:28:22 by teando           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	try {
		if (argc != 2) {
			throw std::invalid_argument("Usage: ./webserv <config.file>");
		}
		Webserv webserv(argv[1]);
		// webserv.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
