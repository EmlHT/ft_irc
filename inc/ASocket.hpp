/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ASocket.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehouot <ehouot@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/04 15:33:22 by ehouot            #+#    #+#             */
/*   Updated: 2024/06/04 18:08:14 by ehouot           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

class ASocket {
protected:

    int sockfd;

public:
    ASocket() : sockfd(-1) {}
    virtual ~ASocket() {
        if (sockfd != -1) {
            close(sockfd);
        }
    }

    int getSocketFd() const { return sockfd; }
};