/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/26 15:49:53 by ehouot           ###   ########.fr       */
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
				parseBuffer((char *)this->_concatBuffer.c_str(),
						this->_pollVec[i].fd, i);
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
			{
				start = i + 2;
				i++;
			}
		}
	}
	if (searchfd(pollVecFd)->getCheckConnection()[0]
			&& searchfd(pollVecFd)->getCheckConnection()[1]
			&& searchfd(pollVecFd)->getCheckConnection()[2]) {
		searchfd(pollVecFd)->setIsConnect();
		welcomeMessages(pollVecFd);
	}
}

void	Server::parseBuffer(char *buffer, int pollVecFd, int index)
{
	std::string str(buffer);

	size_t i;
	std::string firstWord = getFirstWord(str);
	std::string tokensList[11] = {"KICK", "INVITE", "TOPIC", "MODE", "QUIT",
		"NICK", "USER", "PASS", "PRIVMSG", "JOIN", "PART"};
	int (Server::*function_table[11])(std::string buffer, int pollVecFd,
			int index) = {&Server::cmdKick,
		&Server::cmdInvite, &Server::cmdTopic, &Server::cmdMode,
		&Server::cmdQuit, &Server::cmdNick, &Server::cmdUser, &Server::cmdPass,
		&Server::cmdPrivsmg, &Server::cmdJoin, &Server::cmdPart};
	for (i = 0; i < sizeof(tokensList) / sizeof(tokensList[0]); i++)
	{
		if (tokensList[i].compare(firstWord) == 0)
		{
			std::string bufferRest = "";
			if (firstWord.size() + 1 <= str.size())
				bufferRest = str.substr(firstWord.size() + 1, std::string::npos);
			(this->*function_table[i])(bufferRest, pollVecFd, index);
			break ;
		}
	}
	if (i == 11)
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "421"
				+ " " + searchfd(pollVecFd)->getNick()
				+ " " + firstWord + " " + ":Unknown command" + "\r\n");
}

int	Server::needMoreParams(std::string buffer, ClientSocket* client)
{
	if (buffer == "") {
		std::string notEnoughParamMessage = ":" + std::string(SERV_NAME) + " 461 "
			+ client->getNick() + " PRIVMSG :Not enough parameters" + "\r\n";
		client->sendMessage(notEnoughParamMessage);
		return 461;
	}
	return 0;
}

int Server::findClientSocketFd(std::vector<ClientSocket*>& vec, const std::string& targetNick) {
	for (std::vector<ClientSocket*>::const_iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if ((*it)->getNick() == targetNick || (*it)->getUserName() == targetNick)
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

int	Server::cmdKick(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd)) == 461)
		return (0);

	std::string channelName = getFirstWord(buffer), user = getSecondWord(buffer), reason = getRemainingWords(buffer, 2);
	std::vector<std::string> users;
	size_t pos = 0, coma;
	while ((coma = user.find(",", pos)) != std::string::npos) {
		users.push_back(user.substr(pos, coma - pos));
		pos = coma + 1;
	}
	users.push_back(user.substr(pos));
	if (size_t doubleP = reason.find(":") != std::string::npos && (doubleP == 0))
		reason = reason.substr(1);

	Channel* channel = findChannelName(_channelSocket, channelName);
	if (!channel)
	{
		std::string noChannelMessage = std::string(SERV_NAME) + " 403 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :No such channel";
		searchfd(pollVecFd)->sendMessage(noChannelMessage);
		return (0);
	}
	if (!channel->isOperator(searchfd(pollVecFd)))
	{
		std::string notOperatorMessage = std::string(SERV_NAME) + " 482 " + channelName + " :You're not channel operator";
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
			std::string noSuchNickMessage = std::string(SERV_NAME) + " 401 " + users[i] + " :No such nick/channel";
			searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
			continue;
		}
		if (!channel->isMember(userToKick))
		{
			std::string notOnChannelMessage = std::string(SERV_NAME) + " 441 " + users[i] + " " + channelName + " :They aren't on that channel";
			searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
		}
		std::string kickMessage;
		if (reason == "")
		{
			kickMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " KICK " + channelName + " " + users[i] + " :" + users[i];
			reason = "";
		}
		else
			kickMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " KICK " + channelName + " " + users[i] + " :" + reason;
		channel->broadcastMessage(kickMessage);
		channel->deleteUser(userToKick);
	}
	return (0);
}

int	Server::cmdInvite(std::string buffer, int pollVecFd, int index) {
	return (0);
}

int	Server::cmdTopic(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd)) == 461)
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
						std::string topicMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " TOPIC " + channelName + " :" + topic;
						channel->broadcastMessage(topicMessage);
					}
					else
					{
						std::string notOperatorMessage = std::string(SERV_NAME) + " 482 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :You're not channel operator";
						searchfd(pollVecFd)->sendMessage(notOperatorMessage);
					}
				}
				break;
			}
			if (!clientIsOnChannel)
			{
				std::string notOnChannelMessage = std::string(SERV_NAME) + " 442 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :You're not on that channel";
				searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
			}
		}
	}
	if (!channelExists)
	{
		std::string noChannelMessage = std::string(SERV_NAME) + " 403 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :No such channel";
		searchfd(pollVecFd)->sendMessage(noChannelMessage);
	}
	return (0);
}

int	Server::cmdMode(std::string buffer, int pollVecFd, int index) {
//	std::cout << buffer << std::endl;
	Channel	*chan = findChannelName(_channelSocket, getFirstWord(buffer));
	if (!chan)
	{
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "403"
				+ " " + searchfd(pollVecFd)->getNick() + " " + getFirstWord(buffer)
				+ " " + ":No such channel" + "\r\n");
		return (1);
	}
	if (getSecondWord(buffer).compare("") == 0) {
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "324"
			+ " " + searchfd(pollVecFd)->getNick() + " " + getFirstWord(buffer)
			+ " " + chan->activeModes() + "\r\n");
		searchfd(pollVecFd)->sendMessage(":" + std::string(SERV_NAME) + " " + "329"
			+ " " + searchfd(pollVecFd)->getNick() + " " + getFirstWord(buffer)
			+ " " + chan->getCreateTime() + "\r\n");
	}
	if (getSecondWord(buffer).compare("+i") == 0
			|| getSecondWord(buffer).compare("-i") == 0)
		std::cout << "Option i" << std::endl;
	if (getSecondWord(buffer).compare("+t") == 0
			|| getSecondWord(buffer).compare("-t") == 0)
		std::cout << "Option t" << std::endl;
	if (getSecondWord(buffer).compare("+k") == 0
			|| getSecondWord(buffer).compare("-k") == 0)
		std::cout << "Option k" << std::endl;
	if (getSecondWord(buffer).compare("+o") == 0
			|| getSecondWord(buffer).compare("-o") == 0)
		std::cout << "Option o" << std::endl;
	if (getSecondWord(buffer).compare("+l") == 0
			|| getSecondWord(buffer).compare("-l") == 0)
		std::cout << "Option l" << std::endl;
	return (0);
}

int	Server::cmdQuit(std::string buffer, int pollVecFd, int index) {
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

int	Server::cmdPrivsmg(std::string buffer, int pollVecFd, int index)
{
	if (needMoreParams(buffer, searchfd(pollVecFd)) == 461)
		return (0);
	std::string target = getFirstWord(buffer), text = getSecondWord(buffer);

	if (text == "")
	{
		std::string NoTextMessage = ":" + std::string(SERV_NAME) + " 412 " + searchfd(pollVecFd)->getNick() + " PRIVMSG :No text to send" + "\r\n";
 		searchfd(pollVecFd)->sendMessage(NoTextMessage);
		return (0);
	}
	std::vector<std::string> targets;
	size_t pos = 0, coma;
	while ((coma = target.find(",", pos)) != std::string::npos) {
        targets.push_back(target.substr(pos, coma - pos));
        pos = coma + 1;
    }
    targets.push_back(target.substr(pos));

	if (text.find(":") == 0)
		text = text.substr(1);
	for (std::vector<std::string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		if (!it->empty() && it->at(0) == '#')
		{
			Channel *channel = findChannelName(_channelSocket, *it);
			if (channel)
				channel->broadcastMessage(text);
			else
			{
				std::string noSuchChanMessage = ":" + std::string(SERV_NAME) + " 401 " + _channelSocket.at(index)->getName() + " " + *it + " PRIVMSG :No such channel" + "\r\n";
				searchfd(pollVecFd)->sendMessage(noSuchChanMessage);
			}
		}
		else 
		{
			int targetFd = findClientSocketFd(_clientSocket, *it);
			if (targetFd != -1)
				send(targetFd, text.c_str(), text.size(), 0);
			else
			{
				std::string noSuchNickMessage = ":" + std::string(SERV_NAME) + " 401 " + searchfd(pollVecFd)->getNick() + " " + *it + " PRIVMSG :No such nick" + "\r\n";
				searchfd(pollVecFd)->sendMessage(noSuchNickMessage);
			}
		}
	}
	return (0);
}

int	Server::cmdJoin(std::string buffer, int pollVecFd, int index)
{
	if (needMoreParams(buffer, searchfd(pollVecFd)) == 461)
		return (0);
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

	if (searchfd(pollVecFd)->getNbJoinChannels() >= 10)
	{
		std::string tooManyChannelsMessage = ":" + std::string(SERV_NAME) + " 405 " + searchfd(pollVecFd)->getNick() + " " + targets[0] + ":You have joined too many channels" + "\r\n";
		searchfd(pollVecFd)->sendMessage(tooManyChannelsMessage);
		return (0);
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
				channel->addUser(searchfd(pollVecFd), channelPassword);
				break;
			}
		}
		if (!channelExists)
		{
			channel = new Channel(channelName, channelPassword);
			channel->addUser(searchfd(pollVecFd), channelPassword);
			if (channel->getListClients().empty())
			{
				delete channel;
				return (0);
			}
			_channelSocket.push_back(channel);
			channel->setOperator(searchfd(pollVecFd));
        }
        std::string joinMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " JOIN " + channelName + "\r\n";
        channel->broadcastMessage(joinMessage);

		if (!channel->getTopic().empty())
		{
			std::stringstream ss;
			ss << channel->getTopicSetAt();
			std::string topicSetAtStr = ss.str();

			std::string topicMessage = ":" + std::string(SERV_NAME) + " 332 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
			searchfd(pollVecFd)->sendMessage(topicMessage);

			std::string topicWhoTimeMessage = ":" + std::string(SERV_NAME) + " 333 " + searchfd(pollVecFd)->getNick() + " " + channelName + " " + channel->getTopicSetBy() + " " + topicSetAtStr + "\r\n";
			searchfd(pollVecFd)->sendMessage(topicWhoTimeMessage);
        }
		std::string modeMessage = ":" + std::string(SERV_NAME) + " MODE " + channelName + " " + channel->activeModes() + "\r\n";
		searchfd(pollVecFd)->sendMessage(modeMessage);

		std::string namesMessage = ":" + std::string(SERV_NAME) + " 353 " + searchfd(pollVecFd)->getNick() + " @ " + channelName + " :";
		std::vector<ClientSocket*> clients = channel->getListClients();
		for (size_t j = 0; j < clients.size(); ++j) {
			namesMessage += (j == 0 ? "" : " ") + clients[j]->getNick();
		}
		searchfd(pollVecFd)->sendMessage(namesMessage  + "\r\n");

		std::string endNamesMessage = ":" + std::string(SERV_NAME) + " 366 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :End of /NAMES list" + "\r\n";
		searchfd(pollVecFd)->sendMessage(endNamesMessage);

		for (size_t j = 0; j < clients.size(); ++j) {
			if (clients[j] != searchfd(pollVecFd)) {
				clients[j]->sendMessage(joinMessage);
			}
		}
	}
	return (0);
}

int	Server::cmdPart(std::string buffer, int pollVecFd, int index) {
	if (needMoreParams(buffer, searchfd(pollVecFd)) == 461)
		return (0);
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
					if (searchfd(pollVecFd) == (*itl))
					{
						if (reason == "")
							reason = searchfd(pollVecFd)->getNick();
						std::string partMessage = ":" + searchfd(pollVecFd)->getNick() + "!" + searchfd(pollVecFd)->getUserName() + "@" + searchfd(pollVecFd)->getClientIP() + " PART " + channelName + " :" + reason + "\r\n";
        				channel->broadcastMessage(partMessage);
						channel->deleteUser(*itl);
					}
					else
					{
						std::string notOnChannelMessage = std::string(SERV_NAME) + " 442 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :You're not on that channel" + "\r\n";
        				searchfd(pollVecFd)->sendMessage(notOnChannelMessage);
					}
				}
            }
			if (!channelExists)
			{
				std::string noChannelMessage = std::string(SERV_NAME) + " 403 " + searchfd(pollVecFd)->getNick() + " " + channelName + " :No such channel" + "\r\n";
        		searchfd(pollVecFd)->sendMessage(noChannelMessage);
        	}
		}
	}
	return (0);
}

int	Server::_buffer_recv_limit = 512;
