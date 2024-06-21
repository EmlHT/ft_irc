/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/21 16:21:40 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <stdio.h>

Server::Server(unsigned short port, std::string password) : _port(port),
	_password(password)
{
}

Server::~Server()
{
}

void	Server::checkPort(char *port) const
{
	if (strlen(port) > 5)
		throw PortProblem();

	int	i = 0;

	while (port[i])
	{
		if (port[i] < '0' || port[i] > '9')
			throw PortProblem();
		i++;
	}

	if (atoi(port) != _port)
		throw PortProblem();

	if (_port > USHRT_MAX)
		throw PortProblem();

	if (port[0] == '0' && port[1] == '\0')
		throw PortProblem();
}

void	Server::checkPassword(char *password) const
{
	int	i = 0;

	while (password[i])
	{
		if (password[i] < '!' || password[i] > '~')
			throw PasswordProblem();
		i++;
	}

	if (this->_password.size() > 128)
		throw PasswordProblem();
}

void	Server::initServer()
{
	if (!_listener.ListenAndBind(this->_port))
		throw NotListenableOrBindable();
	addInStructPollfd(_listener.getSocketFd(), POLLIN);
	while (true)
	{
		int nbPollRevent = poll(_pollVec.data(), _pollVec.size(), -1);
		if (nbPollRevent < 0) {
			std::cerr << errno << std::endl;
			break;
		}
		for (size_t i = 0; i < _pollVec.size(); i++)
		{
			if (_pollVec[i].revents & POLLIN)
			{
				if (_pollVec[i].fd == _listener.getSocketFd()) //acceptNewClient
				{
					int client_socket = _listener.AcceptConnection();
					if (client_socket < 0)
						continue ;
					if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
					{
						std::cerr << errno << std::endl;
						continue ;
					}
					_clientSocket.push_back(new ClientSocket(client_socket));
					addInStructPollfd(client_socket, POLLIN);
					std::cout << "Connection Done !" << std::endl;
				}
				else //clientTreats
				{
					char	*bufferContent = (char *) searchfd(_pollVec[i].fd)->getBuffer();
					ssize_t bytes_received = recv(_pollVec[i].fd, (void *) bufferContent, Server::_buffer_recv_limit - 1/*sizeof(bufferContent) - 1*/, 0);
					if (bytes_received <= 0) {
						if (bytes_received < 0) {
							std::cerr << errno << std::endl;
						}
						clientSocketEraser(_pollVec[i].fd);
						close(_pollVec[i].fd);
						_pollVec.erase(_pollVec.begin() + i);
						--i;
						std::cout << "Client closed" << std::endl;
					}
					else
					{
						bufferContent[bytes_received] = '\0';
						if (isTerminatedByN(bufferContent) == 0)
							this->_concatBuffer += std::string(bufferContent);
						else if (!(std::string(bufferContent).compare("\n") == 0
								|| std::string(bufferContent).compare("\r\n") == 0))
						{
							this->_concatBuffer += std::string(bufferContent);
							if (isTerminatedByN(bufferContent) == 1)
								this->_concatBuffer = this->_concatBuffer.substr(0, this->_concatBuffer.size() - 2);
							else if (isTerminatedByN(bufferContent) == 2)
								this->_concatBuffer = this->_concatBuffer.substr(0, this->_concatBuffer.size() - 1);
							if (searchfd(_pollVec[i].fd)->getIsConnect() == false)
								this->firstConnection((char *)this->_concatBuffer.c_str(), _pollVec[i].fd, i);
							else
								parseBuffer((char *)this->_concatBuffer.c_str(), _pollVec[i].fd, i);
							this->_concatBuffer.clear();
						}
					}
				}
			}
		}
	}
}

void	Server::clientSocketEraser(int fd)
{
	std::vector<ClientSocket*>::iterator it = _clientSocket.begin();
	std::vector<ClientSocket*>::iterator ite = _clientSocket.end();
	for (it = _clientSocket.begin(); it != ite; it++)
	{
		if (fd == (*it)->getSocketFd())
		{
			delete *it;
			_clientSocket.erase(it);
		}
	}
}

void	Server::addInStructPollfd(int fd, short event)
{
	pollfd NewPoll;
	NewPoll.fd = fd;
	NewPoll.events = event;
	_pollVec.push_back(NewPoll);
}

ClientSocket	*Server::searchfd(int fd) const
{
	std::vector<ClientSocket*>::const_iterator it = _clientSocket.begin();
	std::vector<ClientSocket*>::const_iterator ite = _clientSocket.end();
	for (it = _clientSocket.begin(); it != ite; it++)
	{
		if (fd == (*it)->getSocketFd())
			return (*it);
	}
	return NULL;
}

std::string getFirstWord(const std::string& str)
{
	std::istringstream iss(str);
	std::string firstWord;
	if (!(iss >> firstWord)) {
		return "";
	}
	return firstWord;
}

std::string getSecondWord(const std::string& str)
{
	std::istringstream iss(str);
	std::string firstWord, secondWord;
	iss >> firstWord;
	if (!(iss >> secondWord)) {
		return "";
	}
	return secondWord;
}

size_t	Server::isTerminatedByN(char *buffer) const {
	size_t	len = strlen(buffer);
	if (len > 1 && buffer[len - 2] == '\r' && buffer[len - 1] == '\n')
		return 1;
	if (len > 0 && buffer[len - 1] == '\n')
		return 2;
	return 0;
}

void	Server::firstConnection(char *buffer, int pollVecFd, int index)
{
	size_t	start = 0;
	std::string str(buffer);

	for (size_t i = 0; i < strlen(buffer); ++i)
	{
		if ((buffer[i] == '\r' && buffer[i + 1] == '\n')
				|| buffer[i + 1] == '\0')
		{
			if (getFirstWord(&str[start]).compare("PASS") == 0)
				cmdPass((char *)str.substr(start, i + 1 - start).c_str(),
						pollVecFd, index);
			if (buffer[i] == '\r' && buffer[i + 1] == '\n')
			{
				start = i + 2;
				i++;
			}
		}
	}
}

void	Server::parseBuffer(char *buffer, int pollVecFd, int index)
{
	std::string str(buffer);

	size_t i;
	std::string firstWord = getFirstWord(str);
	std::string tokensList[11] = {"KICK", "INVITE", "TOPIC", "MODE", "QUIT",
		"NICK", "USER", "PASS", "PRIVMSG", "JOIN", "PART"};
	void (Server::*function_table[11])(std::string buffer, int pollVecFd, int index) = {&Server::cmdKick,
		&Server::cmdInvite, &Server::cmdTopic, &Server::cmdMode,
		&Server::cmdQuit, &Server::cmdNick, &Server::cmdUser, &Server::cmdPass,
		&Server::cmdPrivsmg, &Server::cmdJoin, &Server::cmdPart};
	for (i = 0; i < sizeof(tokensList) / sizeof(tokensList[0]); i++)
	{
		if (tokensList[i].compare(firstWord) == 0)
		{
			std::string bufferRest = str.substr(firstWord.size() + 1);
			(this->*function_table[i])(bufferRest, pollVecFd, index);
			break ;
		}
	}
	if (i == 11)
		searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME) + " " + "421"
				+ " " + searchfd(pollVecFd)->getNick()
				+ " " + firstWord + " " + ":Unknown command" + "\r\n");
}

int	Server::needMoreParams(std::string buffer, ClientSocket* client)
{
	if (buffer == "") {
		std::cout << SERV_NAME << " 461 " << client->getNick() << " PRIVMSG :Not enough parameters" << std::endl;
		return 461;
	}
	return 0;
}

int Server::findClientSocketFd(std::vector<ClientSocket*>& vec, const std::string& targetNick) {
    for (std::vector<ClientSocket*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if ((*it)->getNick() == targetNick || (*it)->getName() == targetNick)
			return (*it)->getSocketFd();
	}
	return -1;
}

Channel* Server::findChannelName(std::vector<Channel*>& vec, const std::string& targetName) {
	for (std::vector<Channel*>::const_iterator it = vec.begin(); it != vec.end(); ++it) 
	{
		if ((*it)->getName() == targetName)
			return (*it);
	}
	return NULL;
}

void	Server::cmdKick(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdInvite(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdTopic(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(pollVecFd);
	if (needMoreParams(buffer, _clientSocket.at(index)) == 461)
		return;
	std::string channelName = getFirstWord(buffer), topic = getSecondWord(buffer);
	bool channelExists = false;
	Channel* channel;
    for (std::vector<Channel*>::iterator it = _channelSocket.begin(); it != _channelSocket.end(); ++it)
	{
		if (channelName == (*it)->getName())
		{
			channelExists = true;
			channel = *it;
			std::vector<ClientSocket*> listClient = channel->getListClients();
			for (std::vector<ClientSocket*>::iterator itl = listClient.begin(); itl != listClient.end(); ++itl)
			{
				if (_clientSocket.at(index) == (*itl))
				{
					if (channel->isOperator(_clientSocket.at(index)))
					{
						channel->setTopic(topic);
						std::string topicMessage = ":" + _clientSocket.at(index)->getNick() + "!" + _clientSocket.at(index)->getName() + "@" + _clientSocket.at(index)->getClientIP() + " TOPIC " + channelName + " :" + topic;
        				channel->broadcastMessage(topicMessage);
						break;
					}
					else
					{
						std::string notOperatorMessage = std::string(SERV_NAME) + " 482 " + _clientSocket.at(index)->getNick() + " " + channelName + " :You're not channel operator";
        				_clientSocket.at(index)->sendMessage(notOperatorMessage);
					}
				}
				else
				{
					std::string notOnChannelMessage = std::string(SERV_NAME) + " 442 " + _clientSocket.at(index)->getNick() + " " + channelName + " :You're not on that channel";
        			_clientSocket.at(index)->sendMessage(notOnChannelMessage);
				}
			}
		}
		if (!channelExists)
		{
			std::string noChannelMessage = std::string(SERV_NAME) + " 403 " + _clientSocket.at(index)->getNick() + " " + channelName + " :No such channel";
        	_clientSocket.at(index)->sendMessage(noChannelMessage);
        }
	}
	
}

void	Server::cmdMode(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdQuit(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdNick(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdUser(std::string buffer, int pollVecFd, int index) {
}

void	Server::cmdPass(std::string buffer, int pollVecFd, int index) {
	if (getSecondWord(buffer) == "")
		searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME) + " " + "461"
				+ " " + searchfd(pollVecFd)->getNick() + " " + "PASS"
				+ " " + ":Not enough parameters" + "\r\n");
	else if (searchfd(_pollVec[index].fd)->getCheckConnection()[0] == true) {
		searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME)
				+ " " + "462" + " " + searchfd(pollVecFd)->getNick()
				+ " " + ":You may not reregister" + "\r\n");
	}
	else
	{
		int	i = 0;
		while (getSecondWord(buffer).c_str()[i]) {
			if (getSecondWord(buffer).c_str()[i] < '!'
					|| getSecondWord(buffer).c_str()[i] > '~') {
				searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME)
						+ " " + "464" + " " + searchfd(pollVecFd)->getNick()
						+ " " + ":Password incorrect" + "\r\n");
				searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME)
						+ " " + searchfd(pollVecFd)->getNick()
						+ " " + "ERROR" + " " + ":Connection timeout" + "\r\n");
				clientSocketEraser(pollVecFd);
				close(pollVecFd);
				_pollVec.erase(_pollVec.begin() + index);
				return ;
			}
			i++;
		}
		if (getSecondWord(buffer).compare(_password) != 0)
		{
			searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME) + " " + "464"
					+ " " + searchfd(pollVecFd)->getNick()
					+ " " + ":Password incorrect" + "\r\n");
			searchfd(pollVecFd)->sendMessage(std::string(SERV_NAME)
					+ " " + searchfd(pollVecFd)->getNick()
					+ " " + "ERROR" + " " + ":Connection timeout" + "\r\n");
			clientSocketEraser(pollVecFd);
			close(pollVecFd);
			_pollVec.erase(_pollVec.begin() + index);
		}
		else
			searchfd(_pollVec[index].fd)->setCheckConnection(true, 0);
	}
}

void	Server::cmdPrivsmg(std::string buffer, int pollVecFd, int index)
{
	static_cast<void>(pollVecFd);
	if (needMoreParams(buffer, _clientSocket.at(index)) == 461)
		return;
	std::string target = getFirstWord(buffer), text = getSecondWord(buffer);

	if (text == "")
	{
		std::string NoTextMessage = std::string(SERV_NAME) + " 412 " + _clientSocket.at(index)->getNick() + "PRIVMSG :No text to send";

 		_clientSocket.at(index)->sendMessage(NoTextMessage);
	}
	std::vector<std::string> targets;
	std::vector<std::string>::iterator it = targets.begin(), ite = targets.end();
	size_t pos = 0, coma;
	while ((coma = target.find(",", pos)) != std::string::npos) {
        targets.push_back(target.substr(pos, coma - pos));
        pos = coma + 1;
    }
    targets.push_back(target.substr(pos));
	
	if (size_t doubleP = text.find(":") != std::string::npos && (doubleP == 0))
		text = text.substr(1);
	for (it = targets.begin(); it != ite; it++)
	{
		if (it[0] == "#")
		{
			if (Channel *channel = findChannelName(_channelSocket, *it))
				channel->broadcastMessage(text);
			else
				std::cout << SERV_NAME << " 401 " << _channelSocket.at(index)->getName() << " " << *it << " PRIVMSG :No such channel" << std::endl;
		}
		else if (int targetFd = findClientSocketFd(_clientSocket, *it) != -1)
			send(targetFd, text.c_str(), text.size(), 0);
		else
			std::cout << SERV_NAME << " 401 " << _clientSocket.at(index)->getNick() << " " << *it << " PRIVMSG :No such nick" << std::endl;
	}
}

void	Server::cmdJoin(std::string buffer, int pollVecFd, int index)
{
	static_cast<void>(pollVecFd);
	if (needMoreParams(buffer, _clientSocket.at(index)) == 461)
		return;
	std::string target = getFirstWord(buffer), pass = getSecondWord(buffer);

	std::vector<std::string> targets;
	std::vector<std::string> passwords;

	size_t pos = 0, coma;
	while ((coma = target.find(",", pos)) != std::string::npos) {
        targets.push_back(target.substr(pos, coma - pos));
        pos = coma + 1;
    }
    targets.push_back(target.substr(pos));

	pos = 0;
	while ((coma = pass.find(",", pos)) != std::string::npos) {
        passwords.push_back(pass.substr(pos, coma - pos));
        pos = coma + 1;
    }
    passwords.push_back(pass.substr(pos));

	if (passwords.size() < targets.size())
        passwords.resize(targets.size());

	if (_clientSocket.at(index)->getNbJoinChannels() >= 10)
	{
		std::cout << SERV_NAME << " 405 " << _clientSocket.at(index)->getNick() << " " << targets[0] << " :You have joined too many channels" << std::endl;
        return;
	}
	for (size_t i = 0; i < targets.size(); ++i)
	{
        std::string channelName = targets[i];
        std::string channelPassword = passwords[i];
        bool channelExists = false;
		Channel* channel;
		
        for (std::vector<Channel*>::iterator it = _channelSocket.begin(); it != _channelSocket.end(); ++it)
		{
            if (channelName == (*it)->getName()) 
			{
                channelExists = true;
				channel = *it;
                channel->addUser(_clientSocket.at(index), channelPassword);
				break;
            }
        }
        if (!channelExists)
		{
            channel = new Channel(channelName, channelPassword);
            channel->addUser(_clientSocket.at(index), channelPassword);
			if (channel->getListClients().empty())
			{
				delete channel;
				return;
			}
            _channelSocket.push_back(channel);
            channel->setOperator(_clientSocket.at(index));
        }
        std::string joinMessage = ":" + _clientSocket.at(index)->getNick() + "!" + _clientSocket.at(index)->getName() + "@" + _clientSocket.at(index)->getClientIP() + " JOIN " + channelName;
        channel->broadcastMessage(joinMessage);

        std::string modeMessage = std::string(SERV_NAME) + " MODE " + channelName + " " + channel->activeModes();
        _clientSocket.at(index)->sendMessage(modeMessage);

        std::string namesMessage = std::string(SERV_NAME) + " 353 " + _clientSocket.at(index)->getNick() + " @ " + channelName + " :";
        std::vector<ClientSocket*> clients = channel->getListClients();
        for (size_t j = 0; j < clients.size(); ++j) {
            namesMessage += (j == 0 ? "" : " ") + clients[j]->getNick();
        }
        _clientSocket.at(index)->sendMessage(namesMessage);

        std::string endNamesMessage = std::string(SERV_NAME) + " 366 " + _clientSocket.at(index)->getNick() + " " + channelName + " :End of /NAMES list";
        _clientSocket.at(index)->sendMessage(endNamesMessage);

        for (size_t j = 0; j < clients.size(); ++j) {
            if (clients[j] != _clientSocket.at(index)) {
                clients[j]->sendMessage(joinMessage);
            }
        }
    }
}

void	Server::cmdPart(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(pollVecFd);
	if (needMoreParams(buffer, _clientSocket.at(index)) == 461)
		return;
	std::string channels = getFirstWord(buffer), reason = getSecondWord(buffer);
		
	std::vector<std::string> channelsList;
	size_t pos = 0, coma;
	while ((coma = channels.find(",", pos)) != std::string::npos) {
        channelsList.push_back(channels.substr(pos, coma - pos));
        pos = coma + 1;
    }
    channelsList.push_back(channels.substr(pos));
	
	for (size_t i = 0; i < channelsList.size(); i++)
	{
		std::string channelName = channelsList[i];
		bool channelExists = false;
		Channel* channel;

        for (std::vector<Channel*>::iterator it = _channelSocket.begin(); it != _channelSocket.end(); ++it)
		{
            if (channelName == (*it)->getName())
			{
                channelExists = true;
				channel = *it;
				std::vector<ClientSocket*> listClient = channel->getListClients();
				for (std::vector<ClientSocket*>::iterator itl = listClient.begin(); itl != listClient.end(); ++itl)
				{
					if (_clientSocket.at(index) == (*itl))
					{
						std::string partMessage = ":" + _clientSocket.at(index)->getNick() + "!" + _clientSocket.at(index)->getName() + "@" + _clientSocket.at(index)->getClientIP() + " PART " + channelName + " :" + reason;
        				channel->broadcastMessage(partMessage);
						channel->deleteUser(*itl);
					}
					else
					{
						std::string notOnChannelMessage = std::string(SERV_NAME) + " 442 " + _clientSocket.at(index)->getNick() + " " + channelName + " :You're not on that channel";
        				_clientSocket.at(index)->sendMessage(notOnChannelMessage);
					}
				}
            }
			if (!channelExists)
			{
				std::string noChannelMessage = std::string(SERV_NAME) + " 403 " + _clientSocket.at(index)->getNick() + " " + channelName + " :No such channel";
        		_clientSocket.at(index)->sendMessage(noChannelMessage);
        	}
		}
	}
}

int	Server::_buffer_recv_limit = 512;
