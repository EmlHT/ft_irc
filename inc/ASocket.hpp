/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:33:22 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/07 12:23:52 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>

class ASocket {
protected:

    int _sockfd;

public:

    ASocket() : _sockfd(-1) {};
    ASocket(int fd) : _sockfd(fd) {};
    virtual ~ASocket() {
        if (_sockfd != -1) {
            close(_sockfd);
        }
    }
    int getSocketFd() const { return _sockfd; }
    void setSocketFd(int fd) { this->_sockfd = fd;};
};
