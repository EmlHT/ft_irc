/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 12:20:20 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 16:54:21 by ehouot           ###   ########.fr       */
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

		std::string _name;
		std::vector<ClientSocket*> _listClients;
		std::string _topic;
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
			std::vector<std::string> _listOperator;
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
		std::vector<ClientSocket*> 	getListClients() const;
		_modes						getModes() const;
		
		void						setTopic(std::string topic);
		void						setPassword(std::string password);
		void						setOperator(ClientSocket* client);
		void						addUser(ClientSocket* client, std::string password);
		
		void						broadcastMessage(std::string &message);
		std::string					activeModes();
		
};
