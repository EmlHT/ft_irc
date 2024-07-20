/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 16:27:26 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:08:56 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ASocket.hpp"
#include "ListenSocket.hpp"
#include "ClientSocket.hpp"

ListenSocket::ListenSocket() : ASocket()
{
}

ListenSocket::~ListenSocket()
{
}

// Try to create a socket to listen and bind the local adress with the socket
bool	ListenSocket::ListenAndBind(int port)
{
	struct sockaddr_in adress_local;
	adress_local.sin_family = AF_INET;
	adress_local.sin_port = htons(port);
	adress_local.sin_addr.s_addr = INADDR_ANY;

	setSocketFd(socket(AF_INET, SOCK_STREAM, 0));
	if (getSocketFd() < 0)
		return false;

	int en = 1;
	if(setsockopt(getSocketFd(), SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) < 0)
		return false;
	if (fcntl(getSocketFd(), F_SETFL, O_NONBLOCK) < 0)
		return false;
	if (bind(getSocketFd(), (struct sockaddr*) &adress_local, sizeof(adress_local)) < 0)
		return false;
	if (listen(getSocketFd(), SOMAXCONN) < 0)
		return false;
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
