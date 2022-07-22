/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 17:51:49 by ablondel          #+#    #+#             */
/*   Updated: 2022/07/21 19:59:24 by ablondel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <vector>
#define BACKLOG 255
#define NBPORTS 3



int     main(void)
{
    std::vector<int> sockets(3);
    std::vector<int> ports(3, 0);
    std::vector<struct sockaddr_in*> addrs(3);
    ports.push_back(8080);
    ports.push_back(8081);
    ports.push_back(8082);
}