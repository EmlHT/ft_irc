/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:29:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/12 12:40:57 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "inc/ASocket.hpp"
#include <vector>
#include <iostream>
#include <string>

class ClientSocket : public ASocket {

	private :
	
		std::string	_password;
		std::string	_userNick;
		std::string	_userName;
		char 		_buffer[1024];
		bool		_isConnect;
		bool		_checkConnection[3];
		
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
		bool 				getIsConnect() const;
		const bool	 		*getCheckConnection() const;
		void 				setNick(std::string nick);
		void 				setName(std::string name);
		void 				setPass(std::string password);
		void 				setIsConnect();
		void	 			setCheckConnection(bool property, int index);
};
