/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:14:04 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:20:16 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <stdio.h>

Channel::Channel(std::string name, std::string password) : _name(name), _channelPass(password), _isPass(!password.empty())
{
	modes._i = false;
	modes._t = true;
	modes._k = false;
	modes._l = false;
}

Channel::~Channel()
{
}

std::string Channel::getName() const
{
	return this->_name;
}

Channel::_modes	Channel::getModes() const
{
	return modes;
}


std::string Channel::getPassword() const
{
	return this->_channelPass;
}

std::vector<ClientSocket*> Channel::getListClients() const
{
	return this->_listClients;
}

void		Channel::setTopic(std::string topic)
{
	this->_topic = topic;
}

void		Channel::setPassword(std::string password)
{
	this->_channelPass = password;
	this->_isPass = !password.empty();
}

void		Channel::addUser(ClientSocket* client, std::string password)
{
	if (this->modes._i)
	{
		if (std::find(modes._listInvited.begin(), modes._listInvited.end(), client->getNick()) == modes._listInvited.end())
		{
			std::cout << SERV_NAME << " 473 " << client->getNick() << " " << this->_name << " JOIN :Cannot join channel (+i)" << std::endl;
			return;
		}
	}
	if (this->modes._k)
	{
		if (this->_channelPass != password)
		{
			std::cout << SERV_NAME << " 475 " << client->getNick() << " " << this->_name << " JOIN :Cannot join channel (+k)" << std::endl;
			return;
		}
	}
	if (this->modes._l && this->_listClients.size() >= static_cast<size_t>(this->modes._limitValue))
	{
		std::cout << SERV_NAME << " 471 " << client->getNick() << " " << this->_name << " JOIN :Cannot join channel (+l)" << std::endl;
		return;
	}
	this->_listClients.push_back(client);
	client->setAddJoinChannels();
}

void		Channel::setOperator(ClientSocket* client)
{
	this->modes._listOperator.push_back(client->getNick());
}

void		Channel::broadcastMessage(std::string &message)
{
	for(int i = 0; i < static_cast<int>(this->_listClients.size()); i++)
	{
		this->_listClients[i]->sendMessage(message);
	}
}

std::string intToString(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string	Channel::activeModes()
{
	std::string modesOn = "+Cns";
	if (this->modes._i)
		modesOn = "+Cins";
	if (this->modes._t)
		modesOn = modesOn + "t";
	if (this->modes._l)
		modesOn = modesOn + "l";
	if (this->modes._k)
		modesOn = modesOn + "k";
	if (this->modes._l)
		modesOn = modesOn + " " + intToString(modes._limitValue);
	if (this->modes._k)
		modesOn = modesOn + " " + _channelPass;
	return (modesOn);
}
