/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/11 16:05:17 by ehouot           ###   ########.fr       */
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

#define SERV_NAME "42.nice.gg"

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
		void	parseBuffer(char *buffer, int pollVecFd);

		void	cmdKick(std::string buffer, int pollVecFd);
		void	cmdInvite(std::string buffer, int pollVecFd);
		void	cmdTopic(std::string buffer, int pollVecFd);
		void	cmdMode(std::string buffer, int pollVecFd);
		void	cmdQuit(std::string buffer, int pollVecFd);
		void	cmdNick(std::string buffer, int pollVecFd);
		void	cmdUser(std::string buffer, int pollVecFd);
		void	cmdPass(std::string buffer, int pollVecFd);
		void	cmdPrivsmg(std::string buffer, int pollVecFd);
		void	cmdJoin(std::string buffer, int pollVecFd);
		void	cmdPart(std::string buffer, int pollVecFd);

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
