/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 16:27:26 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/04 18:30:34 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ListenSocket.hpp"

int		ListenSocket::getFdSocket()
{
	return this->fd_socket;
}

void	ListenSocket::setFdSocket(int socket)
{
	this->fd_socket = socket;
}

// Try to create a socket to listen and bind the local adress with the socket
bool	ListenSocket::ListenAndBind(int port)
{
	setFdSocket(socket(AF_INET, SOCK_STREAM, 0));
	if (getFdSocket() < 0)
	{
		std::cerr << errno << std::endl;
	}
	struct sockaddr_in adress_local;
	adress_local.sin_family = AF_INET;
	adress_local.sin_port = htons(port);
	adress_local.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(getSocketFd(), (struct sockaddr*) &adress_local, sizeof(adress_local)) < 0)
	{
		std::cerr << errno << std::endl;
		return false;
	}
	if (listen(getSocketFd(), 10) < 0) // 10 -> valeur par defaut
	{
		std::cerr << errno << std::endl;
		return false;
	}
	return true;
}

int		ListenSocket::AcceptConnection()
{
	
}
