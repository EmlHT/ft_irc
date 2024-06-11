/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/11 11:22:07 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
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
		void	parseBuffer(char *buffer);

		void	cmdKick(std::string buffer);
		void	cmdInvite(std::string buffer);
		void	cmdTopic(std::string buffer);
		void	cmdMode(std::string buffer);
		void	cmdQuit(std::string buffer);
		void	cmdNick(std::string buffer);
		void	cmdUser(std::string buffer);
		void	cmdPass(std::string buffer);
		void	cmdPrivsmg(std::string buffer);
		void	cmdJoin(std::string buffer);
		void	cmdPart(std::string buffer);

	public :
	
		Server(unsigned short port);
		~Server();

		class NotListenableOrBindable : public std::exception {
			public :
				virtual const char* what() const throw() {
					return strerror(errno);
				}
		};

		class BufferProblem : public std::exception {
			public :
				virtual const char* what() const throw() {
					return strerror(errno);
				}
		};
};
