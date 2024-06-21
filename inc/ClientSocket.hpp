/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:29:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:05:00 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ASocket.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>

class ClientSocket : public ASocket {

	private :

		std::string	_password;
		std::string	_userNick;
		std::string	_userName;
		std::string	_realName;
		char 		_buffer[1024];
		bool		_isConnect;
		bool		_checkConnection[3];
		int			_nbJoinChannels;
		char*		_clientIP;

		ClientSocket();
		ClientSocket(const ClientSocket &src);
		ClientSocket & operator=(const ClientSocket &rhs);

	public :

		ClientSocket(int fd);
		virtual ~ClientSocket();

		const char	 		*getBuffer() const;
		const std::string	getNick() const;
		const std::string	getName() const;
		const std::string	getPass() const;
		const bool	 		*getCheckConnection() const;
		int					getNbJoinChannels() const;
		bool	 			getIsConnect() const;
		const char *		getClientIP() const;

		void 				setNick(std::string nick);
		void 				setName(std::string name);
		void 				setPass(std::string password);
		void 				setIsConnect();
		void	 			setCheckConnection(bool property, int index);
		void				setAddJoinChannels();
		void				setSubJoinChannels();
		void				setClientIP();

		void				sendMessage(const std::string &message);
};
