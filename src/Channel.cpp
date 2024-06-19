/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:14:04 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/19 12:20:04 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/Channel.hpp"

#include <stdio.h>

Channel::Channel(std::string name, std::string password) : _name(name), _channelPass(password), _isPass(!password.empty())
{
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
	if (this->modes._l && this->_listClients.size() >= this->modes._limitValue)
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
