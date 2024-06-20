/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:36:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:08:43 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int fd) : ASocket(fd), _isConnect(false) 
{
	setClientIP();
}

ClientSocket::~ClientSocket()
{
}

/* ---- GETTERS ---- */

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

bool		ClientSocket::getIsConnect() const
{
    return this->_isConnect;
}

int	        ClientSocket::getNbJoinChannels() const
{
    return this->_nbJoinChannels;
}

const char* ClientSocket::getClientIP() const
{
    return this->_clientIP;
}

/* ---- SETTERS ---- */

void        ClientSocket::setNick(std::string nick)
{
    this->_userNick = nick;
}

void        ClientSocket::setName(std::string name)
{
    this->_userName = name;
}

void        ClientSocket::setPass(std::string password)
{
    this->_password = password;
}

void		ClientSocket::setIsConnect()
{
    this->_isConnect = true;
}

void		ClientSocket::setAddJoinChannels()
{
    this->_nbJoinChannels++;
}

void		ClientSocket::setSubJoinChannels()
{
    this->_nbJoinChannels--;
}

void		ClientSocket::setClientIP()
{
	struct sockaddr_storage clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    if (getsockname(this->_sockfd, (struct sockaddr *)&clientAddr, &addrLen) == 0)
	{
        struct sockaddr_in *s = (struct sockaddr_in *)&clientAddr;
		// this->_clientIPMutex.lock();
        this->_clientIP = inet_ntoa(s->sin_addr);
		// this->_clientIPMutex.unlock();
    }
	else
        std::cerr << "Problem with the recuperation of the IP adress of the client" << std::endl;
}

void		ClientSocket::sendMessage(const std::string &message)
{
	const char* msg = message.c_str();
    size_t msg_len = message.length();
    size_t total_bytes_sent = 0;

    while (total_bytes_sent < msg_len)
    {
        ssize_t bytes_sent = send(this->_sockfd, msg + total_bytes_sent, msg_len - total_bytes_sent, 0);
        if (bytes_sent == -1)
        {
            std::cerr << "Erreur lors de l'envoi du message au client " << this->getNick() << ": " << (errno) << std::endl;
            break;
        }
        total_bytes_sent += bytes_sent;
    }
}
