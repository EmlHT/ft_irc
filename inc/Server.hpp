/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/07/18 17:29:52 by mcordes          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <ctime>
#include <poll.h>
#include <errno.h>
#include <limits.h>
#include "Channel.hpp"
#include "ASocket.hpp"
#include "ListenSocket.hpp"
#include "ClientSocket.hpp"

# define NETWORK_NAME "42.nice.gg"
# define SERV_NAME "42.nice.gg"
# define SERV_VERSION "42.nice.gg-1.0"
# define MODE "itkol"
# define MODE_WITH_OPTION "kol"

class Channel;

class Server {

	private :

		std::string					_datetime;
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

		void			clientTreats(int i);
		bool			acceptNewClient();
		void			addInStructPollfd(int fd, short event);
		ClientSocket	*searchfd(int fd) const;
		void			firstConnection(char *buffer, int pollVecFd, int index);
		void			parseBuffer(char *buffer, int pollVecFd, int index);

		int				cmdKick(std::string buffer, int pollVecFd, int index);
		int				cmdInvite(std::string buffer, int pollVecFd, int index);
		int				cmdTopic(std::string buffer, int pollVecFd, int index);
		int				cmdMode(std::string buffer, int pollVecFd, int index);
		int				cmdNick(std::string buffer, int pollVecFd, int index);
		int				cmdUser(std::string buffer, int pollVecFd, int index);
		int				cmdPass(std::string buffer, int pollVecFd, int index);
		int				cmdPrivmsg(std::string buffer, int pollVecFd, int index);
		int				cmdJoin(std::string buffer, int pollVecFd, int index);
		int				cmdPart(std::string buffer, int pollVecFd, int index);
		int				cmdPing(std::string buffer, int pollVecFd, int index);
		int				cmdWho(std::string buffer, int pollVecFd, int index);
		int				cmdWhois(std::string buffer, int pollVecFd, int index);

		int				needMoreParams(std::string buffer, ClientSocket* client, std::string cmd);
		ClientSocket*	clientReturn(std::string nick) const;
		void			clientSocketEraser(int fd);
		size_t			isTerminatedByN(char *buffer) const;
		bool			nameSyntaxChecker(char const *nick) const;
		bool			realNameSyntaxChecker(char const *nick) const;
		bool			nickExist(std::string nick) const;
		std::string		applyChannelModes(Channel* channel, const std::string& modeParams, int pollVecFd);

		int				findClientSocketFd(std::vector<ClientSocket*>& vec, std::string& targetNick);
		Channel*		findChannelName(std::vector<Channel*>& vec, const std::string& targetName);
		void			welcomeMessages(int pollVecFd) const;

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
