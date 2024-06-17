/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/17 17:26:19 by ehouot           ###   ########.fr       */
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
#include "Channel.hpp"
#include <errno.h>

#define SERV_NAME "42.nice.gg"

class Server {

	private :

		unsigned short				_port;
		std::string					_password;
		ListenSocket				_listener;
		std::vector<ClientSocket*>	_clientSocket; // Vecteur des clients connectes
		static int					_buffer_recv_limit;
		std::vector<Channel*> 		_channelSocket; // Vecteur des channels connectes
		std::vector<struct pollfd>	_pollVec;
		Server();
		Server(const Server &src);
		Server & operator=(const Server &rhs);

		void			addInStructPollfd(int fd, short event);
		ClientSocket	*searchfd(int fd) const;
		void			firstConnection(char *buffer, int pollVecFd, int index);
		void			parseBuffer(char *buffer, int pollVecFd, int index);

		void			cmdKick(std::string buffer, int pollVecFd, int index);
		void			cmdInvite(std::string buffer, int pollVecFd, int index);
		void			cmdTopic(std::string buffer, int pollVecFd, int index);
		void			cmdMode(std::string buffer, int pollVecFd, int index);
		void			cmdQuit(std::string buffer, int pollVecFd, int index);
		void			cmdNick(std::string buffer, int pollVecFd, int index);
		void			cmdUser(std::string buffer, int pollVecFd, int index);
		void			cmdPass(std::string buffer, int pollVecFd, int index);
		void			cmdPrivsmg(std::string buffer, int pollVecFd, int index);
		void			cmdJoin(std::string buffer, int pollVecFd, int index);
		void			cmdPart(std::string buffer, int pollVecFd, int index);

		int				needMoreParams(std::string buffer, ClientSocket* client);

		template < typename T >
		int findFdTarget(std::vector<T>& TSockets, const std::string& targetNick) {
			for (typename std::vector<T>::const_iterator it = TSockets.begin(); it != TSockets.end(); ++it)
			{
				if (it->getNick() == targetNick || it->getName() == targetNick)
					return it->getSocketFd();
			}
			return -1;
		}

	public :

		Server(unsigned short port, std::string password);
		~Server();

		void	initServer();
		void	checkPort(char *port) const;
		void	checkPassword(char *password)const;

		class PortProblem : public std::exception {
			public :
				virtual const char* what() const throw() {
					return "Wrong port.";
				}
		};

		class PasswordProblem : public std::exception {
			public :
				virtual const char* what() const throw() {
					return "Wrong password.";
				}
		};

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
