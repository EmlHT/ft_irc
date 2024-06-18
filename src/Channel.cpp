/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:14:04 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/18 18:25:23 by ehouot           ###   ########.fr       */
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

void		Channel::setTopic(std::string topic)
{
	this->_topic = topic;
}

void		Channel::setPassword(std::string password)
{
	this->_channelPass = password;
	this->_isPass = !password.empty();
}

void		Channel::addUser(ClientSocket* client)
{
	if (this->modes._i == true)
	{
		for (std::vector<std::string>::iterator it = modes._listInvited.begin(); it != modes._listInvited.end(); ++it)
		{
			if (client->getNick() == (*it))
			{
				if (this->modes._l == true)
				{
					if (this->_listClients.size() < this->modes._limitValue)
					{
						this->_listClients.push_back(client);
						return;
					}
					else
					{
						std::cout << SERV_NAME << " 471 " << client->getNick() << " " << this->_name << " JOIN :Cannot join channel (+l)" << std::endl;
						return;
					}
				}
			}
		}
		std::cout << SERV_NAME << " 473 " << client->getNick() << " " << this->_name << " JOIN :Cannot join channel (+i)" << std::endl;
	}
}

void		Channel::setOperator(ClientSocket* client)
{
	this->modes._listOperator.push_back(client->getNick());
}
