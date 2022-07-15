#ifndef SOCKET_HPP
#define SOCKET_HPP
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

namespace ft
{
	class sock
	{
		private:
			int	_cnct;
			int _sock;
			struct sockaddr_in _addr;

		public:
			sock(int domain, int service, int protocol, int port, size_t interface);
			int connectToNetwork(int sock, struct sockaddr_in address);
			void checkError(int toTest);
			struct sockaddr_in getAddr(void) const;
			int getCnct(void) const;
			int getSock(void) const;
	};
}

#endif