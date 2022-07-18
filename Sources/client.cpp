/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ablondel <ablondel@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/29 15:32:35 by ablondel          #+#    #+#             */
/*   Updated: 2022/06/29 19:44:27 by ablondel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#define PORT 8000

int main(int ac, const char **av)
{
	int client_socket = 0;
	int ret = 0;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Failed to open client socket\n");
		return -1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
	if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    send(client_socket , av[1] , strlen(av[1]) , 0 );
    printf("MESSAGE SUCCESSFULY SENT\n");
    ret = read( client_socket , buffer, 1024);
    printf("%s\n", buffer);
    return 0;
}
