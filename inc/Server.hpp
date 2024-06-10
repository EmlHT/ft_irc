/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/10 18:05:55 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <poll.h>
#include <limits.h>
#include "ASocket.hpp"
#include "ListenSocket.hpp"
#include "ClientSocket.hpp"
#include <errno.h>


class Server {

	private :

		unsigned short _port;
		ListenSocket _listener;
		std::vector<ClientSocket*> _clientSocket; // Vecteur des clients connectes
		std::vector<struct pollfd> _pollVec;
		Server();
		Server(const Server &src);
		Server & operator=(const Server &rhs);

		void	addInStructPollfd(int fd, short event);
		void	initServer();
		char const	*searchfd(int fd) const;

	public :
	
		Server(unsigned short port);
		~Server();

		class NotListenableOrBindable : public std::exception {
			public :
				virtual const char* what() const throw() {
					return strerror(errno);
				}
		};
};
