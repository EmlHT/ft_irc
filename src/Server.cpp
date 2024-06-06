/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/06 12:51:13 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(unsigned short port) : _port(port)
{
}

Server::~Server()
{
}

void    Server::initServer()
{
	if (!_listener.ListenAndBind(this->_port))
		throw NotListenableOrBindable();
	initStructPollfd(_listener.getSocketFd(), POLLIN);
	while (true)
	{
		int nbPollRevent = poll(_pollVec.data(), _pollVec.size(), -1);
    	if (nbPollRevent < 0) {
        	std::cerr << errno << std::endl;
        break;
    	}
		for (int i = 0; i < _clientSocket.size(); i++)
		{
			if (_pollVec[i].revents & POLLIN)
			{
				if (_pollVec[i].fd == _listener.getSocketFd())
				{
					int client_socket = _listener.AcceptConnection();
					if (client_socket < 0)
						continue ;
					_clientSocket.push_back(new ClientSocket(client_socket));
					initStructPollfd(client_socket, POLLIN);
					std::cout << "Connection Done !" << std::endl;
				}
				else
				{
					char	*bufferContent = (char *) searchfd(_pollVec[i].fd);
					ssize_t bytes_received = recv(_pollVec[i].fd, (void *) bufferContent, sizeof(bufferContent) - 1, 0);
					if (bytes_received <= 0) {
						if (bytes_received < 0) {
							std::cerr << errno << std::endl;
						}
						close(_pollVec[i].fd);
						_pollVec.erase(_pollVec.begin() + i);
						--i;
						std::cout << "Client closed" << std::endl;
					}
					else
					{
						bufferContent[bytes_received] = '\0';
						send(_pollVec[i].fd, (void *) bufferContent, bytes_received, 0);
					}
				}
			}
		}
	}
}

void	Server::initStructPollfd(int fd, short event)
{
    pollfd listen_fd;
    listen_fd.fd = fd;
    listen_fd.events = event;
    _pollVec.push_back(listen_fd);
}

char const	*Server::searchfd(int fd) const
{
	std::vector<ClientSocket*>::const_iterator it = _clientSocket.begin();
	std::vector<ClientSocket*>::const_iterator ite = _clientSocket.end();
	for (it = _clientSocket.begin(); it != ite; it++)
	{
		if (fd == (*it)->getSocketFd())
			return (*it)->getBuffer();
	}
	return NULL;
}
