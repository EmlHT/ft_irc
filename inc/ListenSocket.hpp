/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:55:20 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/20 17:08:15 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ASocket.hpp"

class ListenSocket : public ASocket {

	private :

		ListenSocket(const ListenSocket &src);
		ListenSocket & operator=(const ListenSocket &rhs);

	public :

		ListenSocket();
		virtual ~ListenSocket();
		bool	ListenAndBind(int port);
		int		AcceptConnection();
};
