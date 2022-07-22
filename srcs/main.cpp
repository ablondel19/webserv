/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 17:51:49 by ablondel          #+#    #+#             */
/*   Updated: 2022/07/22 20:16:42 by ablondel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define GREEN 		"\e[92m"
#define RED			"\033[31m"
#define RESET   	"\033[0m"

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
#define log(c, x, msg) std::cout << c << x << msg << RESET;

void    close_all(std::vector<int> &sockets)
{
    for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++)
        close(*(it));    
}

int     set_server(std::vector<int> &sockets, std::vector<int> &ports, std::vector<struct sockaddr_in> &addrs)
{
    int on = 1;
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
            std::cerr << "FAIL: while adding NONBLOCK flag to the current socket" << std::endl;
            close_all(sockets);
			return -2;
        }
		if (setsockopt(sockets[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
            std::cerr << "FAIL: while setting up the socket for re-use" << std::endl;
            close_all(sockets);
			return -3;
        }
		memset(&addrs[i], 0, sizeof(addrs[i]));
		addrs[i].sin_family = AF_INET;
		addrs[i].sin_addr.s_addr = inet_addr("127.0.0.1");
		addrs[i].sin_port = htons(ports[i]);
		if ((bind(sockets[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]))) < 0)
        {
            std::cerr << "FAIL: while binding socket to the network" << std::endl;
            close_all(sockets);
			return -4;
        }
		if ((listen(sockets[i], BACKLOG) < 0))
        {
            std::cerr << "FAIL: while listening on the socket for incoming connections" << std::endl;
            close_all(sockets);
			return -5;
        }
	}
	return 0;
}

int     run_server(std::vector<int> &sockets, std::vector<int> &ports, std::vector<struct sockaddr_in> &addrs)
{
    std::vector<int> clients;
    struct timeval timeout;
	unsigned char buffer[30000];
	int current_sd;
	int new_sd;
	int desc_ready;
	int end_server;
    int close_conn;
	int rc;
    int max;
    const char *hello = "HTTP/1.1 200\r\n\r\n<p>Hello World!</p>";
    
    rc = set_server(sockets, ports, addrs);
    if (rc < 0)
        return -1;
    end_server = false;
	fd_set current_sockets;
	fd_set read_sockets;
    FD_ZERO(&current_sockets);
    max = sockets.back();
    for (int i = 0; i < sockets.size(); i++)
    {
        FD_SET(sockets[i], &current_sockets);
        log(GREEN, sockets[i], ": is connected to >> ");
        log(GREEN, ports[i], "\n");
    }
    while (end_server == false)
    {
        timeout.tv_usec = 0;
        timeout.tv_sec = 3 * 60;
        read_sockets = current_sockets;
        rc = select(max + 1, &read_sockets, NULL, NULL, &timeout);
        if (rc < 0)
        {
            log(RED, rc, ": select() failed\n");
            close_all(sockets);
            end_server = true;
            exit(1);
        }
        if (rc == 0)
        {
            log(RED, rc, ": select timed out after 3 min\n");
            close_all(sockets);
            end_server = true;
            exit(1);
        }
        for (int i = 0; i != clients.size(); i++)
        {
            close_conn = false;
			bzero(&buffer, sizeof(buffer)); /* Clear the buffer */
			rc = read(clients[i], buffer, 30000); /* Read data on this connection */
			//DEBUG TO SEE THE HEADER REQUEST//
			//printf("\x1B[32m[[DATA RECEIVED]]\x1B[0m\n\n%s", buffer);
			if (rc < 0)
			{
				break ;
			}
			if (rc == 0)
			{
				break ;
			}
			rc = write(clients[i], hello, strlen(hello)); /* Echo the data to the client */
			if (rc < 0)
			{
				break ;
			}
			if (rc == 0)
			{
				break ;
			}
            close(clients[i]);
			FD_CLR(clients[i], &current_sockets);
        }
        for (int i = 0; i != ports.size(); i++)
        {
            if (FD_ISSET(sockets[i], &read_sockets))
            {
                socklen_t len = sizeof(sockaddr[i]);
                int connection = accept(sockets[i], (struct sockaddr*)&addrs[i], &len);
                log(GREEN, connection, ": connection accepted\n");
                if (connection < 0)
                {
                    log(RED, connection, ": failed to accept incoming connection\n");
                    close_all(sockets);
                    end_server = true;
                }
                FD_SET(connection, &current_sockets);
                clients.push_back(connection);
                log(GREEN, connection, ": added to fd_set\n");
                if (connection > max)
                    max = connection;
                break ;
            }
        }
    }
    close_all(sockets);
    return 0;
}

int     main(void)
{
    std::vector<int> sockets(NBPORTS, 0);
	std::vector<int> ports;
	std::vector<struct sockaddr_in> addrs(NBPORTS);
    ports.push_back(8080);
    ports.push_back(8081);
    ports.push_back(8082);
    run_server(sockets, ports, addrs);
}