/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 16:27:26 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/05 16:10:16 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenSocket.hpp"
#include "ClientSocket.hpp"

ListenSocket::ListenSocket()
{
}

ListenSocket::~ListenSocket()
{
}

// Try to create a socket to listen and bind the local adress with the socket
bool	ListenSocket::ListenAndBind(int port)
{
	setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
	if (getSocketFd() < 0)
	{
		// std::cerr << errno << std::endl;
		return false;
	}
	struct sockaddr_in adress_local;
	adress_local.sin_family = AF_INET;
	adress_local.sin_port = htons(port);
	adress_local.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(getSocketFd(), (struct sockaddr*) &adress_local, sizeof(adress_local)) < 0)
	{
		// std::cerr << errno << std::endl;
		return false;
	}
	if (listen(getSocketFd(), SOMAXCONN) < 0) // 10 -> valeur par defaut
	{
		// std::cerr << errno << std::endl;
		return false;
	}
	return true;
}

int		ListenSocket::AcceptConnection()
{
	int client_sockfd = accept(getSocketFd(), NULL, NULL);
    if (client_sockfd < 0)
	{
        std::cerr << errno << std::endl;
    }
    return client_sockfd;
}
