/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ListenSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:55:20 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/04 17:56:59 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ASocket.hpp"

class ListenSocket : public ASocket {

	private :

		int  fd_socket;

	public :
		
		bool	ListenAndBind(int port);
		int		AcceptConnection();
		void	setFdSocket(int socket);
		int		getFdSocket();
};
