/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot < ehouot@student.42nice.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 15:14:04 by ehouot            #+#    #+#             */
/*   Updated: 2024/07/16 18:31:59 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <stdio.h>

Channel::Channel(std::string name, std::string password) : _name(name), _channelPass(password), _isPass(!password.empty())
{
	modes._i = false;
	modes._t = true;
	modes._k = this->_isPass;
	modes._l = false;

	time_t	now = time(0);
	std::stringstream ss;
	ss << now;
	std::string timestamp = ss.str();
	this->_createTime = timestamp;
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

std::string	Channel::getCreateTime() const
{
	return _createTime;
}

std::string Channel::getPassword() const
{
	return this->_channelPass;
}

std::vector<ClientSocket*> Channel::getListClients() const
{
	return this->_listClients;
}

std::string					Channel::getTopic() const
{
	return this->_topic;
}

std::string					Channel::getTopicSetBy() const
{
	return this->_topicSetBy;
}

time_t					Channel::getTopicSetAt() const
{
	return this->_topicSetAt;
}

void		Channel::setTopicSetBy(std::string &client)
{
	this->_topicSetBy = client;
}
void		Channel::setTopicSetAt()
{
	this->_topicSetAt = time(0);
}

void		Channel::setTopic(std::string topic, std::string client)
{
	this->_topic = topic;
	setTopicSetBy(client);
	setTopicSetAt();
}

void		Channel::setPassword(std::string password)
{
	this->_channelPass = password;
	this->_isPass = !password.empty();
	this->modes._k = this->_isPass;
}

void		Channel::setListInvited(std::string nick)
{
	this->modes._listInvited.push_back(nick);
}

std::string		Channel::addUser(ClientSocket* client, std::string &password)
{
	if (this->modes._i)
	{
		if (std::find(modes._listInvited.begin(), modes._listInvited.end(), client->getNick()) == modes._listInvited.end())
		{
			std::string invRet = ":" + std::string(SERV_NAME) + " 473 " + client->getNick() + " " + this->_name + " :Cannot join channel (+i)\r\n"; 
			return invRet;
		}
	}
	if (this->modes._k)
	{
		if (this->_channelPass != password)
		{
			std::string passRet = ":" + std::string(SERV_NAME) + " 475 " + client->getNick() + " " + this->_name + " :Cannot join channel (+k)\r\n";
			return passRet;
		}
	}
	if (this->modes._l && this->_listClients.size() >= static_cast<size_t>(this->modes._limitValue))
	{
		if (std::find(modes._listInvited.begin(), modes._listInvited.end(), client->getNick()) == modes._listInvited.end())
		{
			std::string listRet = ":" + std::string(SERV_NAME) + " 471 " + client->getNick() + " " + this->_name + " :Cannot join channel (+l)\r\n";
			return listRet;
		}
	}
	this->_listClients.push_back(client);
	client->setAddJoinChannels();
	return "";
}

int		Channel::deleteUser(ClientSocket* client)
{
	std::vector<ClientSocket*>::iterator it = this->_listClients.begin();
	while (it != this->_listClients.end())
	{
		if (client == (*it))
		{
			this->_listClients.erase(it);
			break;
		}
		else
			++it;
	}
	client->setSubJoinChannels();
	removeOperator(client);
	if (this->_listClients.empty())
		return (-1);
	return (0);
}

void		Channel::removeOperator(ClientSocket* client)
{
	std::vector<std::string>::iterator it = this->modes._listOperator.begin();
	while (it != this->modes._listOperator.end())
	{
		if (client->getNick() == (*it))
		{
			this->modes._listOperator.erase(it);
			break;
		}
		else
			++it;
	}
}

void		Channel::setOperator(ClientSocket* client)
{
	if (!isOperator(client))
		this->modes._listOperator.push_back(client->getNick());
}

void		Channel::broadcastPrivmessage(std::string &message, std::string nick)
{
	for(int i = 0; i < static_cast<int>(this->_listClients.size()); i++)
	{
		if (this->_listClients[i]->getNick().compare(nick) != 0)
			this->_listClients[i]->sendMessage(message);
	}
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

bool		Channel::isOperator(ClientSocket* client)
{
	std::vector<std::string>::iterator it = this->modes._listOperator.begin();
	while (it != this->modes._listOperator.end())
	{
		if (client->getNick() == (*it))
			return true;
		else
			++it;
	}
	return false;
}

bool		Channel::isMember(ClientSocket* client)
{
	std::vector<ClientSocket*>::iterator it = this->_listClients.begin();
	while (it != this->_listClients.end())
	{
		if (client == (*it))
			return true;
		else
			++it;
	}
	return false;
}

void	Channel::setInviteOnly(bool activation)
{
	this->modes._i = activation;
}
void	Channel::setTopicRestricted(bool activation)
{
	this->modes._t = activation;
}

void	Channel::removePassword()
{
	this->_channelPass.clear();
	this->modes._k = false;
}
void	Channel::setUserLimit(int limit)
{
	this->modes._limitValue = limit;
	this->modes._l = true;
}
void	Channel::removeUserLimit()
{
	this->modes._limitValue = 0;
	this->modes._l = false;
}
