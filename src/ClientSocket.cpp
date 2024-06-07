/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientSocket.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/05 13:36:18 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/07 12:12:58 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "inc/ClientSocket.hpp"

ClientSocket::ClientSocket(int fd) : ASocket(fd)
{
}

ClientSocket::~ClientSocket()
{
}

const char	*ClientSocket::getBuffer() const
{
    return this->_buffer;
}
