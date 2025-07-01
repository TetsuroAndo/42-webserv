/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:10:43 by hirwatan          #+#    #+#             */
/*   Updated: 2025/06/30 17:22:15 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Webserv.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
	try {
		if (argc != 2) {
			throw std::invalid_argument("usage: ./webserv <config.file>");
		}
		Webserv webserv(argv[1]);
		webserv.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
