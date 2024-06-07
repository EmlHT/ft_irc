/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 14:04:40 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/07 12:11:51 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>
#include "inc/Server.hpp"

unsigned short charToUShort(const char *str)
{
    unsigned short result = 0;
    std::istringstream iss(str);
    iss >> result;
    return result;
}

int main(int argc, char **argv)
{
	if (argc != 3)
    {
        std::cerr << "There must be two parameters." << std::endl;
        return 1;
    }
    unsigned short port = charToUShort(argv[1]);
    if (port > USHRT_MAX && strlen(argv[1]) > 5)
    {
        std::cerr << "Wrong port." << std::endl;
        return 1;
    }
    try {
        Server server(port);
    } catch (std::exception &e) {
        std::cerr << errno << std::endl;
    }
}
