/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/07/15 12:03:34 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <stdio.h>

Server::Server(unsigned short port, std::string password) : _port(port),
	_password(password)
{
	std::time_t	time;
	std::tm* 	timeinfo;

	std::time(&time);
	timeinfo = std::gmtime(&time);

	char	buffer[80];
	strftime(buffer, sizeof(buffer), "%a %b %d %Y at %H:%M:%S UTC", timeinfo);

	this->_datetime = buffer;
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
	if (!this->_listener.ListenAndBind(this->_port))
		throw NotListenableOrBindable();
	addInStructPollfd(this->_listener.getSocketFd(), POLLIN);
	while (true)
	{
		int nbPollRevent = poll(this->_pollVec.data(), this->_pollVec.size(), -1);
		if (nbPollRevent < 0) {
			std::cerr << errno << std::endl;
			break;
		}
		for (size_t i = 0; i < this->_pollVec.size(); i++)
		{
			if (this->_pollVec[i].revents & POLLIN)
			{
				if (this->_pollVec[i].fd == this->_listener.getSocketFd())
				{
					if (!this->acceptNewClient(i))
						continue;
				}
				else
					this->clientTreats(i);
			}
		}
	}
}

bool	Server::acceptNewClient(int i)
{
	int client_socket = this->_listener.AcceptConnection();
	if (client_socket < 0)
		return false;
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << errno << std::endl;
		return false;
	}
	this->_clientSocket.push_back(new ClientSocket(client_socket));
	addInStructPollfd(client_socket, POLLIN);
	std::cout << "Connection Done !" << std::endl;
	return true;
}

void	Server::clientTreats(int i)
{
//	int	j = 0;
//
	char	*bufferContent = (char *) searchfd(this->_pollVec[i].fd)->getBuffer();
	ssize_t bytes_received = recv(this->_pollVec[i].fd, (void *) bufferContent,
			Server::_buffer_recv_limit - 1/*sizeof(bufferContent) - 1*/, 0);
	if (bytes_received <= 0) {
		if (bytes_received < 0)
			std::cerr << errno << std::endl;
		clientSocketEraser(this->_pollVec[i].fd);
		close(this->_pollVec[i].fd);
		this->_pollVec.erase(this->_pollVec.begin() + i);
		--i;
		std::cout << "Client closed" << std::endl;
	}
	else
	{
		bufferContent[bytes_received] = '\0';
//		std::cout << "|||" << bufferContent << "<<<" << std::endl;
		if (isTerminatedByN(bufferContent) == 0)
			this->_concatBuffer += std::string(bufferContent);
		else if (!(std::string(bufferContent).compare("\n") == 0
				|| std::string(bufferContent).compare("\r\n") == 0))
		{
			this->_concatBuffer += std::string(bufferContent);
			if (isTerminatedByN(bufferContent) == 1)
				this->_concatBuffer = this->_concatBuffer.substr(0,
						this->_concatBuffer.size() - 2);
			else if (isTerminatedByN(bufferContent) == 2)
				this->_concatBuffer = this->_concatBuffer.substr(0,
						this->_concatBuffer.size() - 1);
			if (searchfd(this->_pollVec[i].fd)->getIsConnect() == false)
				this->firstConnection((char *)this->_concatBuffer.c_str(),
						this->_pollVec[i].fd, i);
			else
			{
//				std::cout << ">>>>>>>>>>>>>>>>>>>>>>" << this->_concatBuffer.c_str()<< "<<<" << std::endl;
//				std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
//				while (bufferContent[j])
//				{
//					printf("|>%c<|%d|\n", bufferContent[j], bufferContent[j]);
//					j++;
//				}
//				std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
				parseBuffer((char *)this->_concatBuffer.c_str(),
						this->_pollVec[i].fd, i);
			}
			this->_concatBuffer.clear();
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

std::string getRemainingWords(const std::string& str, int startWord)
{
	std::istringstream iss(str);
	std::string word;
	for (int i = 0; i < startWord; ++i) {
		iss >> word;
	}
	std::string remaining;
	std::getline(iss, remaining);

	int	i = 0;
	while (remaining[i] == ' ')
		i++;
	remaining = remaining.substr(i);

	return remaining;
}

size_t	Server::isTerminatedByN(char *buffer) const {
	size_t	len = strlen(buffer);
	if (len > 1 && buffer[len - 2] == '\r' && buffer[len - 1] == '\n')
		return 1;
	if (len > 0 && buffer[len - 1] == '\n')
		return 2;
	return 0;
}

void	Server::welcomeMessages(int pollVecFd) const {
	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "001"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + ":Welcome to the " + NETWORK_NAME + " Network, "
			+ searchfd(pollVecFd)->getNick() + "\r\n");

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "002"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + ":Your host is " + SERV_NAME + ", running version "
			+ SERV_VERSION + "\r\n");

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "003"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + ":This server was created "
			+ this->_datetime + "\r\n");

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "004"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + SERV_NAME + " " + SERV_VERSION + " " + MODE
			+ " " + MODE_WITH_OPTION + "\r\n");

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "005"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + "CASEMAPPING=ascii CHANLIMIT=#:25 CHANMODES=,ok,l,it"
			+ " " + "CHANNELLEN=50 CHANTYPES=#& MODES=5 NETWORK=42.nice.gg"
			+ " " + "NICKLEN=16 PREFIX=(ov)@+ STATUSMSG=@+"
			+ " " + "TARGMAX=KICK:1,PRIVMSG:4 TOPICLEN=307 USERLEN=12"
			+ " " + ":are supported by this server" + "\r\n");
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
			if (getFirstWord(&str[start]).compare("PASS") == 0
					&& cmdPass(getSecondWord(str.substr(start, i + 1 - start)),
						pollVecFd, index))
				return ;
			else if (searchfd(_pollVec[index].fd)->getCheckConnection()[0]
					&& getFirstWord(&str[start]).compare("USER") == 0)
				cmdUser((char *)str.substr(start, i + 1 - start).c_str() + 5,
						pollVecFd, index);
			else if (searchfd(_pollVec[index].fd)->getCheckConnection()[0]
					&& getFirstWord(&str[start]).compare("NICK") == 0)
				cmdNick(getSecondWord(str.substr(start, i + 1 - start)),
						pollVecFd, index);
			if (buffer[i] == '\r' && buffer[i + 1] == '\n')
			{ start = i + 2; i++; } } } if (searchfd(pollVecFd)->getCheckConnection()[0] && searchfd(pollVecFd)->getCheckConnection()[1]
			&& searchfd(pollVecFd)->getCheckConnection()[2]) {
		searchfd(pollVecFd)->setIsConnect();
		welcomeMessages(pollVecFd);
	}
}

void    Server::parseBuffer(char *buffer, int pollVecFd, int index)
{
//	std::cout << ">>>" << buffer << "<<<" << std::endl;
    std::string str(buffer);
    std::vector<std::string> buffParts;
    size_t pos = 0, endOfLine;
    while ((endOfLine = str.find("\r\n", pos)) != std::string::npos) {
        buffParts.push_back(str.substr(pos, endOfLine - pos));
        pos = endOfLine + 2;
    }
    buffParts.push_back(str.substr(pos));
    std::cout << "BUFFER : " << str << std::endl;

//	int	j;
    for (std::vector<std::string>::iterator it = buffParts.begin(); it != buffParts.end(); it++)
    {
        size_t i;
        std::string firstWord = getFirstWord(*it);
//		std::cout << "String: " << (*it) << std::endl;
//		j = 0;
//		while ((*it)[j])
//		{
//			printf("|%c|%d|\n", (*it)[j], (*it)[j]);
//			j++;
//		}
        std::string tokensList[12] = {"KICK", "INVITE", "TOPIC", "MODE",
            "NICK", "USER", "PASS", "PRIVMSG", "JOIN", "PART", "PING", "WHO"};
        int (Server::*function_table[12])(std::string buffer, int pollVecFd, int index) = {&Server::cmdKick,
            &Server::cmdInvite, &Server::cmdTopic, &Server::cmdMode,
            &Server::cmdNick, &Server::cmdUser, &Server::cmdPass,
            &Server::cmdPrivmsg, &Server::cmdJoin, &Server::cmdPart,
			&Server::cmdPing, &Server::cmdWho};
        for (i = 0; i < sizeof(tokensList) / sizeof(tokensList[0]); i++)
        {
            if (tokensList[i].compare(firstWord) == 0)
            {
                std::string bufferRest = "";
                if (firstWord.size() + 1 <= (*it).size())
                    bufferRest = (*it).substr(firstWord.size() + 1, std::string::npos);
                (this->*function_table[i])(bufferRest, pollVecFd, index);
                break ;
            }
        }
        if (i == 12)
            searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "421"
                    + " " + searchfd(pollVecFd)->getNick()
                    + " " + firstWord + " " + ":Unknown command" + "\r\n");
    }
}

//void	Server::parseBuffer(char *buffer, int pollVecFd, int index)
//{
////	std::cout << "|||" << buffer << std::endl;
//	std::string str(buffer);
//	std::cout << "BUFFER : " << str << std::endl;
//
//	size_t i;
//	std::string firstWord = getFirstWord(str);
//	std::string tokensList[11] = {"KICK", "INVITE", "TOPIC", "MODE",
//		"NICK", "USER", "PASS", "PRIVMSG", "JOIN", "PART", "PING"};
//	int (Server::*function_table[11])(std::string buffer, int pollVecFd, int index) = {&Server::cmdKick,
//		&Server::cmdInvite, &Server::cmdTopic, &Server::cmdMode,
//		&Server::cmdNick, &Server::cmdUser, &Server::cmdPass,
//		&Server::cmdPrivmsg, &Server::cmdJoin, &Server::cmdPart, &Server::cmdPing};
//	for (i = 0; i < sizeof(tokensList) / sizeof(tokensList[0]); i++)
//	{
//		if (tokensList[i].compare(firstWord) == 0)
//		{
//			std::string bufferRest = "";
//			if (firstWord.size() + 1 <= str.size())
//				bufferRest = str.substr(firstWord.size() + 1, std::string::npos);
//			(this->*function_table[i])(bufferRest, pollVecFd, index);
//			break ;
//		}
//	}
//	if (i == 11)
//		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "421"
//				+ " " + searchfd(pollVecFd)->getNick()
//				+ " " + firstWord + " " + ":Unknown command" + "\r\n");
//}

int	Server::needMoreParams(std::string buffer, ClientSocket* client, std::string cmd)
{
	if (buffer == "") {
		std::string notEnoughParamMessage = ":" + std::string(SERV_NAME) + " 461 "
			+ client->getNick() + " " + cmd + " :Not enough parameters" + "\r\n";
		client->sendMessage(notEnoughParamMessage);
		return 461;
	}
	return 0;
}

int Server::findClientSocketFd(std::vector<ClientSocket*>& vec, std::string& targetNick) {
	for (std::vector<ClientSocket*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
	{
		std::string nick;

		if ((*it)->getNick()[0] == '@')
			nick = (*it)->getNick().substr(1);
		else
			nick = (*it)->getNick();
		if (nick == targetNick || (*it)->getUserName() == targetNick) {
			nick.clear();
			return (*it)->getSocketFd();
		}
		nick.clear();
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

int	Server::cmdKick(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("KICK")) == 461)
		return (0);

	std::string channelName = getFirstWord(buffer), user = getSecondWord(buffer), reason = getRemainingWords(buffer, 2);
	std::vector<std::string> users;
	size_t pos = 0, coma;
	while ((coma = user.find(",", pos)) != std::string::npos) {
		users.push_back(user.substr(pos, coma - pos));
		pos = coma + 1;
	}
	users.push_back(user.substr(pos));

	size_t doubleP = reason.find(":");
	if (doubleP != std::string::npos && (doubleP == 0))
		reason = reason.substr(1);
	else
		reason = getFirstWord(reason);

	Channel* channel = findChannelName(_channelSocket, channelName);
	if (!channel)
	{
		std::string noChannelMessage = ":" + std::string(SERV_NAME) + " 403 "
			+ searchfd(pollVecFd)->getNick() + " " + channelName
			+ " :No such channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(noChannelMessage);
		return (0);
	}
	if (!channel->isOperator(searchfd(pollVecFd)))
	{
		std::string notOperatorMessage = ":" + std::string(SERV_NAME) + " 482 "
			+ searchfd(pollVecFd)->getNick() + " " + channelName
			+ " :You're not channel operator" + "\r\n";
		searchfd(pollVecFd)->sendMessage(notOperatorMessage);
		return (0);
	}
	for (size_t i = 0; i < users.size(); i++)
	{
		ClientSocket* userToKick = NULL;
		for (std::vector<ClientSocket*>::iterator it = _clientSocket.begin(); it != _clientSocket.end(); ++it) {
			if ((*it)->getNick() == users[i] || (*it)->getUserName() == users[i])
			{
				userToKick = *it;
				break;
			}
		}
		if (!userToKick)
		{
			std::string noSuchNickMessage = ":" + std::string(SERV_NAME)
				+ " 401 " + searchfd(pollVecFd)->getNick() + " " + users[i]
				+ " :No such nick/channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
			continue;
		}
		if (!channel->isMember(userToKick))
		{
			std::string notOnChannelMessage = ":" + std::string(SERV_NAME)
				+ " 441 " + searchfd(pollVecFd)->getNick() + " " + users[i]
				+ " " + channelName + " :They aren't on that channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
			return 1;
		}
		std::string kickMessage;
		if (reason == "")
		{
			kickMessage = ":" + searchfd(pollVecFd)->getNick() + "!"
				+ searchfd(pollVecFd)->getUserName() + "@"
				+ searchfd(pollVecFd)->getClientIP() + " KICK " + channelName
				+ " " + users[i] + " :" + users[i] + "\r\n";
			reason = "";
		}
		else
			kickMessage = ":" + searchfd(pollVecFd)->getNick() + "!"
				+ searchfd(pollVecFd)->getUserName() + "@"
				+ searchfd(pollVecFd)->getClientIP() + " KICK " + channelName
				+ " " + users[i] + " :" + reason + "\r\n";
		channel->broadcastMessage(kickMessage);
		channel->deleteUser(userToKick);
	}
	return (0);
}

int	Server::cmdInvite(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("INVITE")) == 461)
		return (0);
	std::string userName = getFirstWord(buffer), channelName = getSecondWord(buffer);
	if (channelName == "")
	{
		std::string notEnoughParamMessage = ":" + std::string(SERV_NAME)
			+ " 461 " + searchfd(pollVecFd)->getNick() + " "
			+ " INVITE :Not enough parameters" + "\r\n";
		searchfd(pollVecFd)->sendMessage(notEnoughParamMessage);
		return (0);
	}
	Channel* channel = findChannelName(_channelSocket, channelName);
	if (!channel)
	{
		std::string noSuchChanMessage = ":" + std::string(SERV_NAME) + " 403 "
			+ searchfd(pollVecFd)->getNick() + " " + channelName + " :No such channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(noSuchChanMessage);
		return (0);
	}
	ClientSocket* user = NULL;
	for (std::vector<ClientSocket*>::iterator it = _clientSocket.begin(); it != _clientSocket.end(); ++it)
	{
		if ((*it)->getNick() == userName)
		{
			user = *it;
			break;
		}
	}
	if (!user)
	{
		std::string noSuchNickMessage = ":" + std::string(SERV_NAME) + " 401 "
			+ searchfd(pollVecFd)->getNick() + " " + userName + " :No such nick/channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
		return (0);
	}
	if (channel->isMember(user))
	{
		std::string userInChannelMessage = ":" + std::string(SERV_NAME)
			+ " 443 " + searchfd(pollVecFd)->getNick() + " " + userName + " "
			+ channelName + " :is already on channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(userInChannelMessage);
		return (0);
	}
	channel->setListInvited(user->getNick());
	std::string inviteMessage = ":" + std::string(SERV_NAME) + " 341 "
		+ searchfd(pollVecFd)->getNick() + " " + userName + " " + channelName
		+ "\r\n";
	searchfd(pollVecFd)->sendMessage(inviteMessage);
	std::string invitedMessage = ":" + searchfd(pollVecFd)->getNick() + "!"
		+ searchfd(pollVecFd)->getUserName() + "@"
		+ searchfd(pollVecFd)->getClientIP() + " INVITE " + userName 
		+ " :" + channelName + " \r\n";
	searchfd(findClientSocketFd(_clientSocket, userName))->sendMessage(invitedMessage);
	return (0);
}

int	Server::cmdTopic(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("TOPIC")) == 461)
		return (0);
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
			bool clientIsOnChannel = false;
			for (std::vector<ClientSocket*>::iterator itl = listClient.begin(); itl != listClient.end(); ++itl)
			{
				if (searchfd(pollVecFd) == (*itl))
				{
					clientIsOnChannel = true;
					if (channel->isOperator(searchfd(pollVecFd)))
					{
						channel->setTopic(topic, searchfd(pollVecFd)->getNick());
						std::string topicMessage = ":"
							+ searchfd(pollVecFd)->getNick()
							+ "!" + searchfd(pollVecFd)->getUserName() + "@"
							+ searchfd(pollVecFd)->getClientIP() + " TOPIC "
							+ channelName + " :" + topic + "\r\n";
						channel->broadcastMessage(topicMessage);
					}
					else
					{
						std::string notOperatorMessage = ":"
							+ std::string(SERV_NAME) + " 482 "
							+ searchfd(pollVecFd)->getNick() + " "
							+ channelName + " :You're not channel operator"
							+ "\r\n";
						searchfd(pollVecFd)->sendMessage(notOperatorMessage);
					}
					break;
				}
			}
			if (!clientIsOnChannel)
			{
				std::string notOnChannelMessage = ":" + std::string(SERV_NAME)
					+ " 442 " + searchfd(pollVecFd)->getNick() + " "
					+ channelName + " :You're not on that channel" + "\r\n";
				searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
			}
		}
	}
	if (!channelExists)
	{
		std::string noChannelMessage = ":" + std::string(SERV_NAME) + " 403 "
			+ searchfd(pollVecFd)->getNick() + " " + channelName
			+ " :No such channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(noChannelMessage);
	}
	return (0);
}

bool Server::applyChannelModes(Channel* channel, const std::string& modeParams) {
	std::istringstream iss(modeParams);
	char sign = '+';
	std::string key;
	int limit;
	bool expectKey = false;
    bool expectLimit = false;
	char c;

	while (iss >> c)
	{
		if (c == '+' || c == '-') 
			sign = c;
		else if (expectKey) 
		{
			iss >> key;
			channel->setPassword(key);
			expectKey = false;
		}
		else if (expectLimit)
		{
			iss >> limit;
			channel->setUserLimit(limit);
			expectLimit = false;
		}
		else
		{
			switch (c) {
				case 'i':
					if (sign == '+')
						channel->setInviteOnly(true);
					else
						channel->setInviteOnly(false);
					break;
				case 't':
					if (sign == '+')
						channel->setTopicRestricted(true);
					else
						channel->setTopicRestricted(false);
					break;
				case 'k':
					if (sign == '+')
						expectKey = true;
					else
						channel->removePassword();
					break;
				case 'l':
					if (sign == '+')
						expectLimit = true;
					else 
						channel->removeUserLimit();
					break;
				default:
					return false;
			}
		}
	}
	return true;
}

int	Server::cmdMode(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("MODE")) == 461)
		return (0);

	std::string target = getFirstWord(buffer);
	std::string modeParams = buffer.substr(buffer.find(" ") + 1);

	if (target.empty())
	{
		std::string notEnoughParamMessage = ":" + std::string(SERV_NAME) + " 461 " + searchfd(pollVecFd)->getNick() + " MODE :Not enough parameters\r\n";
		searchfd(pollVecFd)->sendMessage(notEnoughParamMessage);
		return (0);
	}
	if (target[0] == '#')
	{
		Channel* channel = findChannelName(_channelSocket, target);
		if (!channel) {
			std::string noSuchChanMessage = ":" + std::string(SERV_NAME) + " 403 " + target + " :No such channel\r\n";
			searchfd(pollVecFd)->sendMessage(noSuchChanMessage);
			return (0);
		}
		if (!channel->isMember(searchfd(pollVecFd))) {	
			std::string notOnChannelMessage = ":" + std::string(SERV_NAME) + " 442 " + searchfd(pollVecFd)->getNick() + " " + target + " :You're not on that channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
			return (0);
		}
		if (!channel->isOperator(searchfd(pollVecFd))) {
			std::string notChanOpMessage = ":" + std::string(SERV_NAME) + " 482 " + target + " :You're not channel operator\r\n";
			searchfd(pollVecFd)->sendMessage(notChanOpMessage);
			return (0);
		}
		if (applyChannelModes(channel, modeParams))
		{
			std::string	msg = ":" + searchfd(pollVecFd)->getNick() + "!"
				+ searchfd(pollVecFd)->getUserName() + "@"
				+ searchfd(pollVecFd)->getClientIP() + " MODE " + target
				+ " " + modeParams + "\r\n";
			channel->broadcastMessage(msg);
			msg.clear();
		}
		else
		{
			if (getSecondWord(buffer).compare("") == 0) {
				searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "324"
					+ " " + searchfd(pollVecFd)->getNick() + " " + getFirstWord(buffer)
					+ " " + channel->activeModes() + "\r\n");
				searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "329"
					+ " " + searchfd(pollVecFd)->getNick() + " " + getFirstWord(buffer)
					+ " " + channel->getCreateTime() + "\r\n");
			}
			else
			{
				std::string modeErrorMessage = ":" + std::string(SERV_NAME) + " 472 " + modeParams + " :is unknown mode char to me\r\n";
				searchfd(pollVecFd)->sendMessage(modeErrorMessage);
			}
		}
	}
	else
	{
		ClientSocket* user = NULL;
		for (std::vector<ClientSocket*>::iterator it = _clientSocket.begin(); it != _clientSocket.end(); ++it) {
			if ((*it)->getNick() == target) {
				user = *it;
				break;
			}
		}
		if (!user) {
			std::string noSuchNickMessage = ":" + std::string(SERV_NAME) + " 401 " + target + " :No such nick/channel\r\n";
			searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
			return (0);
		}
	}
	return (0);
}

bool	Server::nameSyntaxChecker(char const *nick) const {
	if (*nick >= '0' && *nick <= '9')
		return (false);
	while (*nick) {
		if (*nick < '0' || (*nick > '9' && *nick < 'A')
				|| (*nick > ']' && *nick < 'a') || *nick > '}') {
			return (false);
		}
		nick++;
	}
	return (true);
}

bool	Server::nickExist(std::string nick) const {
	std::vector<ClientSocket*>::const_iterator it = _clientSocket.begin();
	std::vector<ClientSocket*>::const_iterator ite = _clientSocket.end();
	for (it = _clientSocket.begin(); it != ite; it++)
	{
		if (nick.compare((*it)->getNick()) == 0 )
			return (false);
	}
	return (true);
}

int	Server::cmdNick(std::string buffer, int pollVecFd, int index) {
	if (getFirstWord(buffer).c_str()[0] == ':')
		buffer = getFirstWord(buffer).substr(1);
	else
		buffer = getFirstWord(buffer);
	if (buffer.compare(searchfd(pollVecFd)->getNick()) == 0)
		return (1);
	if (buffer.size() == 0) {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "431"
				+ " " + searchfd(pollVecFd)->getNick()
				+ " " + ":No nickname given" + "\r\n");
		return (1);
	}
	if (!nameSyntaxChecker(buffer.c_str())) {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "432"
				+ " " + searchfd(pollVecFd)->getNick() + " " + buffer
				+ " " + ":Erroneus nickname" + "\r\n");
		return (1);
	}
	if (!nickExist(buffer)) {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "433"
				+ " " + searchfd(pollVecFd)->getNick() + " " + buffer
				+ " " + ":Nickname is already in use" + "\r\n");
		return (1);
	}
	if (searchfd(pollVecFd)->getIsConnect())
		searchfd(pollVecFd)->sendMessage(":" + searchfd(pollVecFd)->getNick()
				+ "!" + searchfd(pollVecFd)->getUserName() + "@" +
				searchfd(pollVecFd)->getClientIP()
				+ " NICK :" + buffer + "\r\n");
	searchfd(pollVecFd)->setNick(buffer);
	searchfd(_pollVec[index].fd)->setCheckConnection(true, 1);
	return (0);
}

bool	Server::realNameSyntaxChecker(char const *nick) const {
	if (*nick >= '0' && *nick <= '9')
		return (false);
	while (*nick) {
		if (*nick < ' ' || (*nick > ' ' && *nick < '0')
				|| (*nick > '9' && *nick < 'A') || (*nick > ']' && *nick < 'a')
				|| *nick > '}')
			return (false);
		nick++;
	}
	return (true);
}

int	Server::cmdUser(std::string buffer, int pollVecFd, int index) {
	if (searchfd(pollVecFd)->getCheckConnection()[2]) {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
				+ " " + "462" + " " + searchfd(pollVecFd)->getNick()
				+ " " + ":You may not reregister" + "\r\n");
		return (1);
	}
	if (getFirstWord(buffer) == "") {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
				+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
				+ " " + ":Not enough parameters" + "\r\n");
		return (1);
	} else {
		if (buffer.c_str()[getFirstWord(buffer).size()] == '\0') {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		}
		if (!nameSyntaxChecker(getFirstWord(buffer).c_str())) {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		}
		std::string userName = "~" + getFirstWord(buffer);
		buffer = buffer.substr(getFirstWord(buffer).size() + 1, std::string::npos);
		if (buffer.c_str()[getFirstWord(buffer).size()] == '\0') {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		}
		buffer = buffer.substr(getFirstWord(buffer).size() + 1, std::string::npos);
		if (buffer.c_str()[getFirstWord(buffer).size()] == '\0') {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		}
		buffer = buffer.substr(getFirstWord(buffer).size() + 1, std::string::npos);
		if (buffer[0] == ':' && !realNameSyntaxChecker(buffer.c_str() + 1)) {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		} else if (buffer[0] != ':' && !nameSyntaxChecker(getFirstWord(buffer).c_str())) {
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
					+ " " + searchfd(pollVecFd)->getNick() + " " + "USER"
					+ " " + ":Not enough parameters" + "\r\n");
			return (1);
		}
		std::string	realName;
		if (buffer[0] == ':')
			realName = buffer.substr(1, std::string::npos);
		else
			realName = getFirstWord(buffer);
		searchfd(pollVecFd)->setUserName(userName);
		searchfd(pollVecFd)->setRealName(realName);
		searchfd(_pollVec[index].fd)->setCheckConnection(true, 2);
	}
	return (0);
}

int	Server::cmdPass(std::string buffer, int pollVecFd, int index) {
	if (buffer.c_str()[0] == ':')
		buffer = buffer.substr(1);
	if (buffer == "")
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "461"
				+ " " + searchfd(pollVecFd)->getNick() + " " + "PASS"
				+ " " + ":Not enough parameters" + "\r\n");
	else if (searchfd(_pollVec[index].fd)->getCheckConnection()[0] == true)
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
				+ " " + "462" + " " + searchfd(pollVecFd)->getNick()
				+ " " + ":You may not reregister" + "\r\n");
	else
	{
		int	i = 0;
		while (buffer.c_str()[i]) {
			if (buffer.c_str()[i] < '!'
					|| buffer.c_str()[i] > '~') {
				searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
						+ " " + "464" + " " + searchfd(pollVecFd)->getNick()
						+ " " + ":Password incorrect" + "\r\n");
				searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
						+ " " + searchfd(pollVecFd)->getNick()
						+ " " + "ERROR" + " " + ":Connection timeout" + "\r\n");
				clientSocketEraser(pollVecFd);
				close(pollVecFd);
				_pollVec.erase(_pollVec.begin() + index);
				return (1);
			}
			i++;
		}
		if (buffer.compare(_password) != 0)
		{
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "464"
					+ " " + searchfd(pollVecFd)->getNick()
					+ " " + ":Password incorrect" + "\r\n");
			searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
					+ " " + searchfd(pollVecFd)->getNick()
					+ " " + "ERROR" + " " + ":Connection timeout" + "\r\n");
			clientSocketEraser(pollVecFd);
			close(pollVecFd);
			_pollVec.erase(_pollVec.begin() + index);
			return (1);
		}
		else
			searchfd(_pollVec[index].fd)->setCheckConnection(true, 0);
	}
	return (0);
}

int	Server::cmdPrivmsg(std::string buffer, int pollVecFd, int index)
{
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("PRIVMSG")) == 461)
		return (0);
	std::string target = getFirstWord(buffer), text = getRemainingWords(buffer, 1);
	if (text == "")
	{
		std::string NoTextMessage = ":" + std::string(SERV_NAME) + " 412 "
			+ searchfd(pollVecFd)->getNick() + " PRIVMSG :No text to send"
			+ "\r\n";
 		searchfd(pollVecFd)->sendMessage(NoTextMessage);
		return (0);
	}

	if (text.substr(0)[0] == ':')
		text = text.substr(1);
	else
		text = getFirstWord(text);
	text += "\r\n";
	std::vector<std::string> targets;
	size_t pos = 0, coma;
	while ((coma = target.find(",", pos)) != std::string::npos) {
		targets.push_back(target.substr(pos, coma - pos));
		pos = coma + 1;
	}
	std::string	msg;
	targets.push_back(target.substr(pos));
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		if (!it->empty() && it->at(0) == '#')
		{
			Channel *channel = findChannelName(_channelSocket, *it);
			if (channel)
			{
				if (!channel->isMember(searchfd(pollVecFd)))
				{
					std::string msg = ":" + std::string(SERV_NAME)
						+ " 404 " + searchfd(pollVecFd)->getNick() + " " + *it
						+ " :Cannot send to nick/channel" + "\r\n";
					searchfd(pollVecFd)->sendMessage(msg);
					msg.clear();
					return 0;
				}
				msg = ":" + searchfd(pollVecFd)->getNick() + "!"
					+ searchfd(pollVecFd)->getUserName() + "@"
					+ searchfd(pollVecFd)->getClientIP() + " PRIVMSG " + *it
					+ " " + text;
				channel->broadcastPrivmessage(msg, searchfd(pollVecFd)->getNick());
				msg.clear();
			}
			else
			{
				std::string noSuchChanMessage = ":" + std::string(SERV_NAME)
					+ " 401 " + searchfd(pollVecFd)->getNick() + " " + *it
					+ " PRIVMSG :No such channel" + "\r\n";
				searchfd(pollVecFd)->sendMessage(noSuchChanMessage);
			}
		}
		else
		{
			int targetFd = findClientSocketFd(_clientSocket, *it);
			if (targetFd != -1)
			{
				msg = ":" + searchfd(pollVecFd)->getNick() + "!"
					+ searchfd(pollVecFd)->getUserName() + "@"
					+ searchfd(pollVecFd)->getClientIP() + " PRIVMSG " + *it
					+ " " + text;
				send(targetFd, msg.c_str(), msg.size(), 0);
				msg.clear();
			}
			else
			{
				std::string noSuchNickMessage = ":" + std::string(SERV_NAME)
					+ " 401 " + searchfd(pollVecFd)->getNick() + " " + *it
					+ " PRIVMSG :No such nick" + "\r\n";
				searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
			}
		}
	}
	text.clear();
	return (0);
}

int	Server::cmdJoin(std::string buffer, int pollVecFd, int index)
{
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("JOIN")) == 461)
		return (0);
	std::string target = getFirstWord(buffer), pass = getSecondWord(buffer);
	std::vector<std::string> targets;
	std::vector<std::string> passwords;

	size_t pos = 0, coma;
	std::string noChannelMessage;
	while ((coma = target.find(",", pos)) != std::string::npos) {
		if (target.substr(pos, coma - pos)[0] == '#')
			targets.push_back(target.substr(pos, coma - pos));
		else
		{
			noChannelMessage = ":" + std::string(SERV_NAME) + " 403 "
				+ searchfd(pollVecFd)->getNick() + " " + target.substr(pos, coma - pos)
				+ " :No such channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(noChannelMessage);
			noChannelMessage.clear();
		}
		pos = coma + 1;
	}
	if (target.substr(pos)[0] == '#')
		targets.push_back(target.substr(pos));
	else
	{
		noChannelMessage = ":" + std::string(SERV_NAME) + " 403 "
			+ searchfd(pollVecFd)->getNick() + " " + target.substr(pos)
			+ " :No such channel" + "\r\n";
		searchfd(pollVecFd)->sendMessage(noChannelMessage);
		noChannelMessage.clear();
	}

	pos = 0;
	while ((coma = pass.find(",", pos)) != std::string::npos) {
		passwords.push_back(pass.substr(pos, coma - pos));
		pos = coma + 1;
	}
	passwords.push_back(pass.substr(pos));

	if (passwords.size() < targets.size())
		passwords.resize(targets.size());

	// if (searchfd(pollVecFd)->getNbJoinChannels() >= 10)
	// {
	// 	std::string tooManyChannelsMessage = ":" + std::string(SERV_NAME)
	// 		+ " 405 " + searchfd(pollVecFd)->getNick() + " " + targets[0]
	// 		+ ":You have joined too many channels" + "\r\n";
	// 	searchfd(pollVecFd)->sendMessage(tooManyChannelsMessage);
	// 	return (0);
	// }
	for (size_t i = 0; i < targets.size(); ++i)
	{
		std::string joinMessage;
		std::string channelName = targets[i];
		std::string channelPassword = passwords[i];
		bool channelExists = false;
		Channel* channel = NULL;
		for (std::vector<Channel*>::iterator it = _channelSocket.begin(); it != _channelSocket.end(); ++it)
		{
			if (channelName == (*it)->getName())
			{
				channelExists = true;
				channel = *it;
				break;
			}
		}
		if (!channelExists)
		{
			channel = new Channel(channelName, channelPassword);
			_channelSocket.push_back(channel);
			channel->setOperator(searchfd(pollVecFd));
			// if (channel->getListClients().empty())
			// {
			// 	delete channel;
			// 	return (0);
			// }
			// joinMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " JOIN " + channelName + "\r\n";
			// channel->broadcastMessage(joinMessage);
		}
		if (channel->isMember(searchfd(pollVecFd)))
		{
			std::string userInChannelMessage = ":" + std::string(SERV_NAME)
				+ " 443 " + searchfd(pollVecFd)->getNick() + " "
				+ channelName + " :is already on channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(userInChannelMessage);
			continue;
		}
		std::string retAdduser = channel->addUser(searchfd(pollVecFd), channelPassword);
		if (retAdduser != "")
		{
			searchfd(pollVecFd)->sendMessage(retAdduser);
			continue;
		}
		joinMessage = ":" + searchfd(pollVecFd)->getNick() + "!"
			+ searchfd(pollVecFd)->getUserName() + "@"
			+ searchfd(pollVecFd)->getClientIP() + " JOIN " + channelName
			+ "\r\n";
		channel->broadcastMessage(joinMessage);
		if (!channel->getTopic().empty())
		{
			std::stringstream ss;
			ss << channel->getTopicSetAt();
			std::string topicSetAtStr = ss.str();

			std::string topicMessage = ":" + std::string(SERV_NAME) + " 332 "
				+ searchfd(pollVecFd)->getNick() + " " + channelName + " "
				+ channel->getTopic() + "\r\n";
			searchfd(pollVecFd)->sendMessage(topicMessage);

			std::string topicWhoTimeMessage = ":" + std::string(SERV_NAME)
				+ " 333 " + searchfd(pollVecFd)->getNick() + " " + channelName
				+ " " + channel->getTopicSetBy() + "!"
				+ searchfd(pollVecFd)->getUserName() + "@"
				+ searchfd(pollVecFd)->getClientIP() + " "
				+ topicSetAtStr + "\r\n";
			searchfd(pollVecFd)->sendMessage(topicWhoTimeMessage);
		}

		if (channel->getListClients().size() == 1)
		{
			std::string modeMessage = ":" + std::string(SERV_NAME) + " MODE "
				+ channelName + " " + channel->activeModes() + "\r\n";
			searchfd(pollVecFd)->sendMessage(modeMessage);
		}

		std::string namesMessage = ":" + std::string(SERV_NAME) + " 353 "
			+ searchfd(pollVecFd)->getNick() + " @ " + channelName + " :";
		std::vector<ClientSocket*> clients = channel->getListClients();
		for (size_t j = 0; j < clients.size(); ++j)
		{
			if (channel->isOperator(clients[j]))
				namesMessage += (j == 0 ? std::string("") : std::string(" ")) + "@" + clients[j]->getNick();
			else
				namesMessage += (j == 0 ? std::string("") : std::string(" ")) + clients[j]->getNick();
		}
		searchfd(pollVecFd)->sendMessage(namesMessage + "\r\n");

		std::string endNamesMessage = ":" + std::string(SERV_NAME) + " 366 "
			+ searchfd(pollVecFd)->getNick() + " " + channelName
			+ " :End of /NAMES list" + "\r\n";
		searchfd(pollVecFd)->sendMessage(endNamesMessage);
	}
	return (0);
}

int	Server::cmdPart(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd), std::string("PART")) == 461)
		return (0);
	std::string channels = getFirstWord(buffer), reason = getRemainingWords(buffer, 1);

	if (reason.substr(0)[0] == ':')
		reason = reason.substr(1);
	else
		reason = getFirstWord(reason);

	std::vector<std::string> channelsList;
	size_t pos = 0, coma;
	while ((coma = channels.find(",", pos)) != std::string::npos) {
		channelsList.push_back(channels.substr(pos, coma - pos));
		pos = coma + 1;
	}
	channelsList.push_back(channels.substr(pos));
	int	isPart = 0;
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
					if (searchfd(pollVecFd) == (*itl))
					{
						if (reason == "")
							reason = searchfd(pollVecFd)->getNick();
						std::string partMessage = ":" + searchfd(pollVecFd)->getNick()
							+ "!" + searchfd(pollVecFd)->getUserName() + "@"
							+ searchfd(pollVecFd)->getClientIP() + " PART "
							+ channelName + " :" + reason + "\r\n";
						channel->broadcastMessage(partMessage);
						if (channel->deleteUser(*itl) == -1)
						{
							delete channel;
							_channelSocket.erase(it);
						}
						if (i + 1 == channelsList.size())
						{
							reason.clear();
							return (0);
						}
						isPart = 1;
					}
				}
				if (isPart == 0)
				{
					std::string notOnChannelMessage = ":" + std::string(SERV_NAME)
						+ " 442 " + searchfd(pollVecFd)->getNick() + " "
						+ channelName + " :You're not on that channel" + "\r\n";
					searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
				}
				if (i + 1 == channelsList.size())
				{
					reason.clear();
					return (0);
				}
			}
		}
		if (!channelExists && isPart == 0)
		{
			std::string noChannelMessage = ":" + std::string(SERV_NAME) + " 403 "
				+ searchfd(pollVecFd)->getNick() + " " + channelName
				+ " :No such channel" + "\r\n";
			searchfd(pollVecFd)->sendMessage(noChannelMessage);
		}
		isPart = 0;
	}
	reason.clear();
	return (0);
}

int	Server::cmdPing(std::string buffer, int pollVecFd, int index)
{
	std::string	str;

	int	i = 0;
	while (buffer.c_str()[i] == ' ')
		i++;

	str = buffer.substr(i);

	if (str.c_str()[0] == ':')
		str = str.substr(1);
	else
		str = getFirstWord(str);

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME)
			+ " " + "PONG" + " " + std::string(SERV_NAME) + " :" + str + "\r\n");

	str.clear();
	return 0;
}

int	Server::cmdWho(std::string buffer, int pollVecFd, int index)
{
	std::string	str;

	int	i = 0;
	while (buffer.c_str()[i] == ' ')
		i++;

	str = buffer.substr(i);

	if (str.c_str()[0] == ':')
		str = str.substr(1);
	else
		str = getFirstWord(str);

	searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "315"
			+ " " + searchfd(pollVecFd)->getNick()
			+ " " + str + " :End of /WHO list." + "\r\n");

	str.clear();
	return 0;
}

int	Server::_buffer_recv_limit = 512;
