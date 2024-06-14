/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/12 17:53:46 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/Server.hpp"
#include <stdlib.h>

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
		if (password[i] < ' ' || password[i] > '~')
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
//		std::cout << "TEST1 : " << _pollVec.size()<< std::endl;
//		std::cout << "TEST1 : " << _clientSocket.size() << std::endl;
		int nbPollRevent = poll(_pollVec.data(), _pollVec.size(), -1);
		if (nbPollRevent < 0) {
//			std::cout << "TEST : dans if" << std::endl;
			std::cerr << errno << std::endl;
			break;
		}
//		std::cout << "TEST2 : " << _pollVec.size() << std::endl;
//		std::cout << "TEST2 : " << _clientSocket.size() << std::endl;
		for (size_t i = 0; i < _pollVec.size(); i++)
		{
//			std::cout << "TEST : hi" << std::endl;
			if (_pollVec[i].revents & POLLIN)
			{
				if (_pollVec[i].fd == _listener.getSocketFd()) //acceptNewClient
				{
//					std::cout << "TEST : OOO" << std::endl;
//					exit(1);
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
//					std::cout << "TEST : HHH" << std::endl;
//					exit(1);
					char	*bufferContent = (char *) searchfd(_pollVec[i].fd);
					ssize_t bytes_received = recv(_pollVec[i].fd, (void *) bufferContent, Server::_buffer_recv_limit - 1/*sizeof(bufferContent) - 1*/, 0);
//					std::cout << "recv " << bufferContent << std::endl;
//					std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
//					while (*bufferContent)
//					{
//						printf(">%c< %d\n", *bufferContent, *bufferContent);
//						bufferContent++;
//					}
//					std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
					if (bytes_received <= 0) {
						if (bytes_received < 0) {
							std::cerr << errno << std::endl;
						}
						close(_pollVec[i].fd);
						_pollVec.erase(_pollVec.begin() + i);
						--i;
						std::cout << "Client closed" << std::endl;
					}
					else
					{
						bufferContent[bytes_received] = '\0';
						parseBuffer(bufferContent, _pollVec[i].fd, i);
						send(_pollVec[i].fd, (void *) bufferContent, bytes_received, 0); // a mettre en fin de fonctions
//						std::cout << "Send " << bufferContent << std::endl;
					}
				}
			}
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

char const	*Server::searchfd(int fd) const
{
	std::vector<ClientSocket*>::const_iterator it = _clientSocket.begin();
	std::vector<ClientSocket*>::const_iterator ite = _clientSocket.end();
	for (it = _clientSocket.begin(); it != ite; it++)
	{
		if (fd == (*it)->getSocketFd())
			return (*it)->getBuffer();
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
//	if (i == 11)
//		throw BufferProblem();
}

int	Server::needMoreParams(std::string buffer, ClientSocket* client)
{
	if (buffer == "") {
		std::cout << SERV_NAME << " 461 " << client->getNick() << " PRIVMSG :Not enough parameters" << std::endl;
		return 461;
	}
	return 0;
}

void	Server::cmdKick(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdInvite(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdTopic(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdMode(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdQuit(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdNick(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdUser(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPass(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPrivsmg(std::string buffer, int pollVecFd, int index) // <target>{,<target>} <text to be sent>
{
	if (needMoreParams(buffer, _clientSocket.at(index)) == 461) // Check si pas de parametres
		return;
	std::string target = getFirstWord(buffer), text = getSecondWord(buffer);

	if (text == "") // Check si pas de text
		std::cout << SERV_NAME << " 412 " << _clientSocket.at(index)->getNick() << " PRIVMSG :No text to send" << std::endl;

	std::vector<std::string> targets;
	std::vector<std::string>::iterator it = targets.begin(), ite = targets.end();
	size_t pos = 0, coma;
	while (pos != std::string::npos)
	{
		if ((coma = buffer.find("'", pos)) != std::string::npos)
			targets.push_back(buffer.substr(pos, coma - 1));
		pos = coma + 1;
	}
	if (size_t doubleP = text.find(":") != std::string::npos && (doubleP == 0))
		text = text.substr(1);
	for (it; it != ite; it++)
	{
		if (int targetFd = findFdTarget(_clientSocket, *it) != -1)
			send(targetFd, text.c_str(), text.size(), 0);
		else if (int channelFd = findFdTarget(_channelSocket, *it) != -1)
			send(channelFd, text.c_str(), text.size(), 0);
		else
			std::cout << SERV_NAME << " 401 " << _clientSocket.at(index)->getNick() << " " << *it << " PRIVMSG :No such nick/channel" << std::endl;
	}
}

void	Server::cmdJoin(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPart(std::string buffer, int pollVecFd, int index) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

int	Server::_buffer_recv_limit = 512;
