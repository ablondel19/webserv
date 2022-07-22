/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 17:51:49 by ablondel          #+#    #+#             */
/*   Updated: 2022/07/22 13:08:08 by ablondel         ###   ########.fr       */
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
#include "webserv.hpp"

void    close_all(std::vector<int> &sockets)
{
    for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++)
        close(*(it));    
}

int     set_server(std::vector<int> &sockets, std::vector<int> &ports, std::vector<struct sockaddr_in> &addrs)
{
    int on = 1;
    int ret = 0;
	for (size_t i = 0; i < NBPORTS; i++)
	{
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockets[i] < 0)
        {
            std::cerr << "FAIL: while creating socket" << std::endl;
            close_all(sockets);
			return -1;
        }
		if (fcntl(sockets[i], F_SETFL, O_NONBLOCK) < 0)
        {
            std::cerr << "FAIL: while adding NONBLOCK flag" << std::endl;
            close_all(sockets);
			return -2;
        }
		if (setsockopt(sockets[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
            std::cerr << "FAIL: while setting socket for re-use" << std::endl;
            close_all(sockets);
			return -3;
        }
		memset(&addrs[i], 0, sizeof(addrs[i]));
		addrs[i].sin_family = AF_INET;
		addrs[i].sin_addr.s_addr = inet_addr("127.0.0.1");
		addrs[i].sin_port = htons(ports[i]);
		if ((bind(sockets[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]))) < 0)
        {
            std::cerr << "FAIL: while binding socket to network" << std::endl;
            close_all(sockets);
			return -4;
        }
		if ((listen(sockets[i], BACKLOG) < 0))
        {
            std::cerr << "FAIL: while listening to incoming connections" << std::endl;
            close_all(sockets);
			return -5;
        }
	}
	return 0;
}

int     main(void)
{
    std::vector<int> sockets(NBPORTS, 0);
	std::vector<int> ports(NBPORTS);
	std::vector<struct sockaddr_in> addrs(NBPORTS);
    ports.push_back(8080);
    ports.push_back(8081);
    ports.push_back(8082);
    std::cout << set_server(sockets, ports, addrs) << std::endl;
}