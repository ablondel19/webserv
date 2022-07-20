/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/20 13:40:21 by ablondel@st       #+#    #+#             */
/*   Updated: 2022/07/20 17:50:04 by ablondel         ###   ########.fr       */
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
#define SERVER_PORT 8080
#define BACKLOG 32
#define HANDLE_ERROR(x) std::cout << "[" << x << "]" << std::endl;
class webserv
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
        int close_conn;
        int rc;
        int on;
    
    public:
        webserv(const std::string ip, int port, int protocol);
        ~webserv();
};

webserv::webserv(const std::string ip, int port, int protocol)
{
    on = 1;
    end_server = false;
    if ((listen_sd = socket(AF_INET, SOCK_STREAM, protocol)) == -1)
    {
        HANDLE_ERROR(1);
        return ;
    }
    if ((rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))))
    {
        HANDLE_ERROR(2);
        if (!close(listen_sd))
            return ;
        return ;
    }
    if ((rc = fcntl(listen_sd, F_SETFL, fcntl(listen_sd, F_GETFL, 0) | O_NONBLOCK) == -1))
    {
        HANDLE_ERROR(3);
        if (!close(listen_sd))
            return ;
        return ;
    }
    memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);
    if ((rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr) ) == -1))
    {
        HANDLE_ERROR(4);
        if (!close(listen_sd))
            return ;
        return ;
    }
    if ((rc = listen(listen_sd, BACKLOG) == -1))
    {
        HANDLE_ERROR(5);
        if (!close(listen_sd))
            return ;
        return ;
    }
    FD_ZERO(&master_set);
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;
    while (end_server == false)
	{	
		memcpy(&working_set, &master_set, sizeof(master_set)); /* Copy the master fd_set over to the working fd_set */
		rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout); /* Select the first READY file descriptor from the working fd_set */
		if (rc < 0) /* Check to see if the select call failed */
		{
			//perror("  select() failed");
			break ;
		}
		/* Check to see if the 3 minute time out expired. */
		if (rc == 0)
		{
			//ADD REDIRECT TO ERROR PAGE//
			printf("select() timed out. End program.\n");
			exit(1);
		}
		/* One or more descriptors are readable.  Need to determine which ones they are. */
		desc_ready = rc;
		for (current_sd = 0; current_sd <= max_sd  &&  desc_ready > 0; ++current_sd)
		{
			if (FD_ISSET(current_sd, &working_set)) /* Check to see if this descriptor is ready */
			{
				desc_ready -= 1;
				if (current_sd == listen_sd) /* Check to see if this is the listening socket */
				{
					/* Accept all incoming connections */
					while (1)
					{
						new_sd = accept(listen_sd, NULL, NULL);
						if (new_sd < 0)
						{
							//if (errno != EWOULDBLOCK)
							//	end_server = true;
							break ;
						}
						FD_SET(new_sd, &master_set); /* Add the new incoming connection to the master read set */
						if (new_sd > max_sd)
							max_sd = new_sd;
					}
				}
				
				else /* This is not the listening socket, therefore an existing connection must be readable */
				{
					
					close_conn = false; /* Receive all incoming data on this socket */
					while (1)
					{
						rc = read(current_sd, buffer, 30000); /* Read data on this connection */
						//DEBUG TO SEE THE HEADER REQUEST//
						//printf("\x1B[32m[[DATA RECEIVED]]\x1B[0m\n\n%s", buffer);
						if (rc < 0)
						{
							close_conn = true;
							break ;
						}
						if (rc == 0) /* Check to see if the connection has been closed by the client */
						{
							close_conn = true;
							break ;
						}
						/* Echo the data to the client */
						const char *hello = "HTTP/1.1 200OK\r\n\r\n<p>Hello World!</p><script>let nb = 0; console.log(nb);</script>";
						rc = write(current_sd, hello, strlen(hello));
						if (rc < 0)
						{
							//ADD REDIRECT TO ERROR PAGE//
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
}

webserv::~webserv()
{
}

#endif