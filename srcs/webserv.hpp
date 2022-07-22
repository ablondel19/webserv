/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/20 13:40:21 by ablondel@st       #+#    #+#             */
/*   Updated: 2022/07/22 12:50:35 by ablondel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP
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
#define HANDLE_ERROR(x) std::cout << "[" << x << "]" << std::endl;

class client
{
	private:
		struct sockaddr_in addr;
		struct timeval timeout;
		fd_set master_set;
		fd_set working_set;
		unsigned char buffer[30000];
		int current_sd;
		int listen_sd;
		int max_sd;
		int new_sd;
		int desc_ready;
		int end_server;
		int rc;
		int on;
	public:
		client(/* args */) {};
		~client() {};
};

class webserv
{
	public:
		std::vector<int> sockets;
		std::vector<int> ports;
		std::vector<struct sockaddr_in> addrs;

    private:
        struct sockaddr_in addr; //c
        struct timeval timeout; //s
        fd_set master_set;//c
        fd_set working_set;//c
        unsigned char buffer[30000];//c
        int current_sd;//c
        int listen_sd;//c
        int max_sd;//c
        int new_sd;//c
        int desc_ready;//s
        int end_server;//s
        int close_conn;//s
        int rc;//s
        int on;//s
    
    public:
        webserv(int port);
		webserv() {};
        ~webserv();
		int	set_server(std::vector<int> &sockets, std::vector<int> &ports, std::vector<struct sockaddr_in> &addrs)
		{
			for (size_t i = 0; i < NBPORTS; i++)
			{
				if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0) == -1))
					return -1;
				if ((setsockopt(sockets[i], SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == -1))
					return -2;
				if ((fcntl(sockets[i], F_SETFL, O_NONBLOCK) == -1))
					return -3;
				timeout.tv_sec  = 3 * 60;
				timeout.tv_usec = 0;
				memset(&addrs, 0, sizeof(addrs));
				addrs[i].sin_family = AF_INET;
				addrs[i].sin_addr.s_addr = inet_addr("127.0.0.1");
				addrs[i].sin_port = htons(ports[i]);
				if ((bind(sockets[i], (struct sockaddr *)&addrs[i], sizeof(addrs[i]))) == -1)
					return -4;
				if ((listen(sockets[i], BACKLOG) == -1))
					return -5;
			}
			return 0;
		}
};

webserv::webserv(int port)
{
	//SET SERVER////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    on = 1;
    end_server = false;
	const char *hello = "HTTP/1.1 200\r\n\r\n<p>Hello World!</p>";
	listen_sd = -1;
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        return ;
    }
    if ((rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))))
    {
        close(listen_sd);
        return ;
    }
    if ((rc = fcntl(listen_sd, F_SETFL, O_NONBLOCK) == -1))
    {
        close(listen_sd);
        return ;
    }
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(port);
	if ((rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)) == -1))
	{
		close(listen_sd);
		return ;
	}
	if ((rc = listen(listen_sd, BACKLOG) == -1))
	{
		close(listen_sd);
		return ;
	}
	FD_ZERO(&master_set);
	FD_ZERO(&working_set);
	FD_SET(listen_sd, &master_set);
	max_sd = listen_sd;
	//RUN SERVER////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
    while (end_server == false)
	{
		memcpy(&working_set, &master_set, sizeof(master_set)); /* Copy the master fd_set over to the working fd_set */
		rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout); /* Select the first READY I/O file descriptor from the working fd_set */
		if (rc < 0) /* Check to see if the select call failed */
		{
			break ;
		}
		if (rc == 0) /* Check to see if the 3 minute time out expired. */
		{
			//ADD REDIRECT TO ERROR PAGE//
			printf("select() timed out. End program.\n");
			exit(1);
		}
		desc_ready = rc; /* One or more descriptors are readable. Need to determine which ones they are. */
		for (current_sd = 0; current_sd <= max_sd  &&  desc_ready > 0; ++current_sd)
		{
			if (FD_ISSET(current_sd, &working_set)) /* Check to see if this descriptor is ready for I/O */
			{
				desc_ready -= 1;
				if (current_sd == listen_sd) /* Check to see if this is the listening socket */
				{
					while (1) /* Accept all incoming connections */
					{
						new_sd = accept(listen_sd, NULL, NULL);
						if (new_sd < 0)
						{
							break ;
						}
						FD_SET(new_sd, &master_set); /* Add the new incoming connection to the master read set */
						if (new_sd > max_sd)
							max_sd = new_sd;
					}
				}
				else /* This is not the listening socket, therefore an existing connection must be readable */
				{
					close_conn = false;
					while (1)
					{
						bzero(&buffer, sizeof(buffer)); /* Clear the buffer */
						rc = read(current_sd, buffer, 30000); /* Read data on this connection */
						//DEBUG TO SEE THE HEADER REQUEST//
						//printf("\x1B[32m[[DATA RECEIVED]]\x1B[0m\n\n%s", buffer);
						if (rc < 0)
						{
							close_conn = true;
							break ;
						}
						if (rc == 0)
						{
							close_conn = true;
							break ;
						}
						rc = write(current_sd, hello, strlen(hello)); /* Echo the data to the client */
						if (rc < 0)
						{
							close_conn = true;
							break ;
						}
						if (rc == 0)
						{
							close_conn = true;
							break;
						}
					}
					if (close_conn) /* If the close_conn flag was turned on, we need to clean up this active connection */
					{
						close(current_sd);
						FD_CLR(current_sd, &master_set);
						if (current_sd == max_sd)
						{
							while (FD_ISSET(max_sd, &master_set) == false)
								max_sd -= 1;
						}
					}
				}
			}
		}
	}
	for (current_sd = 0; current_sd <= max_sd; ++current_sd)
	{
		if (FD_ISSET(current_sd, &master_set))
			close(current_sd);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

webserv::~webserv()
{

}

#endif