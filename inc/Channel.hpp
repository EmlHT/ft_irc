/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 12:20:20 by ehouot            #+#    #+#             */
/*   Updated: 2024/07/18 14:21:12 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "ClientSocket.hpp"
#include "Server.hpp"

class Channel {

	private :

		std::string	_createTime;
		std::string _name;
		std::vector<ClientSocket*> _listClients;
		std::string _topic;
		std::string _topicSetBy;
		time_t 		_topicSetAt;
		std::string _channelPass;
		bool		_isPass;
		struct _modes
		{
			bool _i;
			std::vector<std::string> _listInvited;
			bool _t;
			bool _k;
			bool _l;
			int _limitValue;
			std::vector<ClientSocket*> _listOperator;
		};

		_modes modes;

		Channel();
		Channel(const Channel &src);
		Channel & operator=(const Channel &rhs);

	public :

		Channel(std::string name, std::string password);
		~Channel();

		std::string 				getName() const;
		std::string 				getPassword() const;
		std::string					getTopic() const;
		std::string					getTopicSetBy() const;
		time_t						getTopicSetAt() const;
		std::vector<ClientSocket*> 	getListClients() const;
		_modes						getModes() const;
		std::string					getCreateTime() const;

		void						setInviteOnly(bool activation);
		void						setUserLimit(int limit);
		void						setListInvited(std::string nick);
		void						removeUserLimit();
		void						setTopicRestricted(bool activation);
		void						setTopic(std::string topic, std::string client);
		void						setTopicSetBy(std::string &client);
		void						setTopicSetAt();
		void						setPassword(std::string password);
		void						removePassword();
		void						setOperator(ClientSocket* client);
		std::string					addUser(ClientSocket* client, std::string &password);
		int							deleteUser(ClientSocket* client);
		void						removeOperator(ClientSocket* client);
		void						setModes(char mode, bool value);

		void						broadcastPrivmessage(std::string &message, std::string nick);
		void						broadcastMessage(std::string &message);
		std::string					activeModes();
		bool						isOperator(ClientSocket* client);
		bool						isMember(ClientSocket* client);

};
