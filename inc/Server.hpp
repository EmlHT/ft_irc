/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:22:35 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include "Channel.hpp"
#include "ASocket.hpp"
#include "ListenSocket.hpp"
#include "ClientSocket.hpp"

# define SERV_NAME "42.nice.gg"

class Channel;

class Server {

	private :

		unsigned short				_port;
		std::string					_password;
		ListenSocket				_listener;
		std::vector<ClientSocket*>	_clientSocket;
		std::vector<Channel*>		_channelSocket;
		static int					_buffer_recv_limit;
		std::vector<struct pollfd>	_pollVec;
		std::string					_concatBuffer;
		
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
		void			clientSocketEraser(int fd);
		size_t			isTerminatedByN(char *buffer) const;

		int				findClientSocketFd(std::vector<ClientSocket*>& vec, const std::string& targetNick);
		Channel*		findChannelName(std::vector<Channel*>& vec, const std::string& targetName);

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
