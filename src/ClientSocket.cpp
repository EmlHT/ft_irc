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

ClientSocket::ClientSocket(int fd) : ASocket(fd), _isConnect(false) 
{
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
