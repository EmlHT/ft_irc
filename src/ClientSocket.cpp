/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:36:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/12 12:43:45 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/ClientSocket.hpp"

/*
 * The privates variables _checkConnection index correspond to:
 * 0 : CAP
 * 1 : PASS
 * 2 : NICK
 * 3 : USER
*/
ClientSocket::ClientSocket(int fd) : ASocket(fd), _isConnect(false) 
{
	this->_checkConnection[0] = false;
	this->_checkConnection[1] = false;
	this->_checkConnection[2] = false;
	this->_checkConnection[3] = false;
}

ClientSocket::~ClientSocket()
{
}

const char	*ClientSocket::getBuffer() const
{
    return this->_buffer;
}

const std::string	ClientSocket::getNick() const
{
    return this->_userNick;
}

const std::string	ClientSocket::getName() const
{
    return this->_userName;
}

const std::string	ClientSocket::getPass() const
{
    return this->_password;
}

bool	ClientSocket::getIsConnect() const
{
    return this->_isConnect;
}

const bool	*ClientSocket::getCheckConnection() const
{
    return this->_checkConnection;
}

void 	ClientSocket::setNick(std::string nick)
{
    this->_userNick = nick;
}

void 	ClientSocket::setName(std::string name)
{
    this->_userName = name;
}

void 	ClientSocket::setPass(std::string password)
{
    this->_password = password;
}

void 	ClientSocket::setIsConnect()
{
    this->_isConnect = true;
}

void	ClientSocket::setCheckConnection(bool property, int index)
{
	this->_checkConnection[index] = property;
}
