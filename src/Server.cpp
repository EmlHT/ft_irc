/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 11:33:19 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/12 12:38:55 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/Server.hpp"

Server::Server(unsigned short port) : _port(port)
{
}

Server::~Server()
{
}

void	Server::initServer()
{
	if (!_listener.ListenAndBind(this->_port))
		throw NotListenableOrBindable();
	addInStructPollfd(_listener.getSocketFd(), POLLIN);
	while (true)
	{
		std::cout << "TEST : hello" << std::endl;
		int nbPollRevent = poll(_pollVec.data(), _pollVec.size(), -1);
		if (nbPollRevent < 0) {
			std::cerr << errno << std::endl;
			break;
		}
		for (size_t i = 0; i < _clientSocket.size(); i++)
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
					char	*bufferContent = (char *) searchfd(_pollVec[i].fd);
					ssize_t bytes_received = recv(_pollVec[i].fd, (void *) bufferContent, sizeof(bufferContent) - 1, 0);
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
						parseBuffer(bufferContent, _pollVec[i].fd);
						send(_pollVec[i].fd, (void *) bufferContent, bytes_received, 0); // a mettre en fin de fonctions

						// Plutot checker le bufferContent et voir ce qu'il y a dedans et le client send plutot que le server.
						// Il faut du coup parser le content, ensuite voir si cela correspond a une commande (on TOKENIZE ???) et cela effectue ou non la commande en question.
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
	iss >> firstWord;
	return firstWord;
}

void	Server::parseBuffer(char *buffer, int pollVecFd)
{
	std::string str(buffer);

	size_t i;
	std::string firstWord = getFirstWord(str);
	std::string tokensList[11] = {"KICK", "INVITE", "TOPIC", "MODE", "QUIT",
		"NICK", "USER", "PASS", "PRIVMSG", "JOIN", "PART"};
	void (Server::*function_table[11])(std::string buffer, int pollVecFd) = {&Server::cmdKick,
		&Server::cmdInvite, &Server::cmdTopic, &Server::cmdMode,
		&Server::cmdQuit, &Server::cmdNick, &Server::cmdUser, &Server::cmdPass,
		&Server::cmdPrivsmg, &Server::cmdJoin, &Server::cmdPart};
	for (i = 0; i < tokensList->size(); i++)
	{
		if (tokensList[i] == firstWord)
		{
			std::string bufferRest = str.substr(firstWord.size() + 1);
			(this->*function_table[i])(bufferRest, pollVecFd);
			break ;
		}
	}
	if (i == 11)
		throw BufferProblem();
}

void	needMoreParams(std::string buffer, int pollVecFd, ClientSocket user)
{
	if (buffer == "\0")
		std::cout << SERV_NAME << " 461 " << user.getNick()/*NICK du USER ici qui doit etre add aux var du user dans de la commande NICK*/ << "PRIVMSG :Not enough parameters" << std::endl; 
	
}

void	Server::cmdKick(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdInvite(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdTopic(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdMode(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdQuit(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdNick(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdUser(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPass(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPrivsmg(std::string buffer, int pollVecFd) {
	static_cast<void>(pollVecFd);
	 
	
}

void	Server::cmdJoin(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}

void	Server::cmdPart(std::string buffer, int pollVecFd) {
	static_cast<void>(buffer);
	static_cast<void>(pollVecFd);
}
