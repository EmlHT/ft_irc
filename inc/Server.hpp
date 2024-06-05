/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:21:25 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/05 18:04:52 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <poll.h>
#include "ListenSocket.hpp"
#include "ClientSocket.hpp"

class Server {

	private :

		int _port;
		ListenSocket _listener;
		std::vector<ClientSocket*> _clientSocket; // Vecteur des clients connectes
		std::vector<struct pollfd> _pollVec;
		Server();
		Server(const Server &src);
		Server & operator=(const Server &rhs);

		void	initStructPollfd(int fd, short event);
		void	initServer();
		char const	*searchfd(int fd) const;

	public :
	
		Server(int port);
		~Server();

		class NotListenableOrBindable : public std::exception {
			public :
				virtual const char* what() const throw() {
					return (const char*)(errno);
				}
		};
};
