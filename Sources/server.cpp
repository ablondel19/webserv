/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/29 15:24:09 by ablondel          #+#    #+#             */
/*   Updated: 2022/06/29 17:48:25 by ablondel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#define PORT 8000

int main(void)
{
	int serverfd;
	int new_socket;
	int ret;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	const char *hello = "Hello World!";
	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(PORT);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));
	bind(serverfd, (struct sockaddr*)&address, sizeof(address));
	listen(serverfd, 10);
	while(1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");
            exit(EXIT_FAILURE);
		}
        char buffer[30000] = {0};
        ret = read( new_socket , buffer, 30000);
        printf("%s\n",buffer );
        write(new_socket , hello , strlen(hello));
        printf("------------------Hello message sent-------------------\n");
        close(new_socket);
    }
	return 0;
}
