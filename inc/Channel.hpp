/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/17 12:20:20 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/17 16:56:10 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <iostream>
#include <string>

class Channel 
{
	private :

		std::string _name;
		std::vector<std::string> _listName;
		std::string _topic;
		std::string _channelPass;
		bool		_isPass;
		struct _modes
		{
			bool _i;
			bool _t;
			bool _k;
			int _l;
			std::vector<std::string> _listOperator;
		};

		Channel(const Channel &src);
		Channel & operator=(const Channel &rhs);
		Channel();

	public :

		Channel (std::string name);
		~Channel();

		std::string getName() const;
		void		setTopic(std::string topic);
		// void		sendToUsers(std::string message);

};
