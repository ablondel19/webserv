/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/21 17:51:49 by ablondel          #+#    #+#             */
/*   Updated: 2022/07/26 12:21:22 by ablondel         ###   ########.fr       */
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
#include <fstream>
#include <sstream>
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
	for (size_t i = 0; i < sockets.size(); i++)
	{
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockets[i] < 0)
        {
            //std::cerr << "FAIL: while creating socket" << std::endl;
            close_all(sockets);
			return -1;
        }
		if (fcntl(sockets[i], F_SETFL, O_NONBLOCK) < 0)
        {
            //std::cerr << "FAIL: while adding NONBLOCK flag to the current socket" << std::endl;
            close_all(sockets);
			return -2;
        }
		if (setsockopt(sockets[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
            //std::cerr << "FAIL: while setting up the socket for re-use" << std::endl;
            close_all(sockets);
			return -3;
        }
		memset(&addrs[i], 0, sizeof(addrs[i]));
		addrs[i].sin_family = AF_INET;
		addrs[i].sin_addr.s_addr = inet_addr("127.0.0.1");
		addrs[i].sin_port = htons(ports[i]);
		if ((bind(sockets[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]))) < 0)
        {
            //std::cerr << "FAIL: while binding socket to the network" << std::endl;
            close_all(sockets);
			return -4;
        }
		if ((listen(sockets[i], BACKLOG) < 0))
        {
            //std::cerr << "FAIL: while listening on the socket for incoming connections" << std::endl;
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
	char buffer[1024];
	int end_server;
    int close_conn;
	int rc;
    int max;
    /////////////////////////////////////////DATA
    std::string ok = "HTTP/1.1 200\r\n\r\n";
    std::string index;
    
    std::ifstream ifs;
    ifs.open("index.html", std::fstream::in);
    while(ifs.read(buffer, sizeof(buffer)))
        index.append(buffer, sizeof(buffer));
    index.append(buffer, ifs.gcount());
    ok.append(index);
    /////////////////////////////////////////
    end_server = false;
    rc = set_server(sockets, ports, addrs);
    if (rc < 0)
        return -1;
	fd_set current_sockets;
	fd_set read_sockets;
    FD_ZERO(&current_sockets);
    max = sockets.back();
    for (std::vector<int>::iterator it = sockets.begin(); it != sockets.end(); it++)
        FD_SET(*it, &current_sockets);
    while (end_server == false)
    {
        FD_ZERO(&read_sockets);
        timeout.tv_usec = 0;
        timeout.tv_sec = 3 * 60;
        read_sockets = current_sockets;
        rc = select(max + 1, &read_sockets, NULL, NULL, &timeout);
        if (rc < 0)
        {
            close_all(sockets);
            end_server = true;
            break ;
        }
        if (rc == 0)
        {
            close_all(sockets);
            end_server = true;
            break ;
        }
        int rd = 0;
        int rw = 0;
        close_conn = false;
        for (std::vector<int>::iterator it = clients.begin(); it != clients.end(); it++)
        {
			bzero(&buffer, sizeof(buffer)); /* Clear the buffer */
            rd = recv(*it, buffer, sizeof(buffer), 0);
            buffer[rd] = 0;
            std::string request(buffer);
			printf("\x1B[32m[[DATA RECEIVED]]\x1B[0m\n\n%s", request.c_str());
			if (rd < 0)
			{
				break ;
			}
			if (rd == 0)
			{
				break ;
			}
            rw = send(*it, ok.c_str(), ok.size(), 0);
			if (rw < 0)
			{
				break ;
			}
			if (rw == 0)
			{
				break ;
			}
            close(*it);
			FD_CLR(*it, &current_sockets);
            clients.erase(it);
        }
        for (size_t i = 0; i < max; i++)
        {
            if (FD_ISSET(sockets[i], &read_sockets))
            {
                socklen_t len = sizeof(sockaddr[i]);
                int connection = accept(sockets[i], (struct sockaddr*)&addrs[i], &len);
                if (connection < 0)
                {
                    close_all(sockets);
                    end_server = true;
                }
                FD_SET(connection, &current_sockets);
                clients.push_back(connection);
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