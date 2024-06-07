/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:29:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/07 12:23:06 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "inc/ASocket.hpp"
#include <iostream>
#include <string>

class ClientSocket : public ASocket {

	private :
		
		char _buffer[1024];
		ClientSocket();
		ClientSocket(const ClientSocket &src);
		ClientSocket & operator=(const ClientSocket &rhs);

	public :

		ClientSocket(int fd);
		virtual ~ClientSocket();
		const char	 *getBuffer() const;
};
