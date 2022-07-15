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
#define PORT 8080
#define NBHOSTS 1

int main(void)
{
	struct sockaddr_in address;
	int server_fd = 0;
	int new_socket = 0;
	int ret = 0;
	int addrlen = sizeof(address);
	char *hello = "KALASH! MOOLAH! KICHTA! KEKRA! De Mesnilmontant a Cambronne j'suis en roue arierre dans l'boul de ta daronne!!";

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Failed to open socket fd\n");
		return -1;
	}
	std::cout << "Socket was openend successfully ont fd: " << server_fd << std::endl;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		perror("Failed to bind the socket\n");
		return -1;
	}
	if (listen(server_fd, NBHOSTS) < 0)
	{
		perror("Failed to listen on socket fd\n");
		return -1;
	}
	while (1)
	{
		std::cout << "WAITING FOR NEW CONNECTIONS..." << std::endl;
		if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
		{
			perror("Failed to accept socket connection\n");
			return -1;
		}
		char buffer[1024] = {0};
		ret = read(new_socket, buffer, 1024);
		buffer[ret] = 0;
		write(new_socket, hello, strlen(hello));
		std::cout << "MESSAGE SUCCESSFULY SENT!" << std::endl;
		close(new_socket);
	}
	return 0;
}
