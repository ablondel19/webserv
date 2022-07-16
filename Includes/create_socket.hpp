#ifndef CREATE_SOCKET_HPP
#define CREATE_SOCKET_HPP

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

class create_socket
{
private:
    int _server_fd; // File descriptor for the socket
    int _domain; // Family of the socket address
    int _type; // Type of service for the socket
    int _protocol; // Specify the available operations for the socket
    struct sockaddr_in _address; // Generic container that allows the OS to read infos

public:
    create_socket(int domain, int type, int protocol);
    ~create_socket() {};
};

/*
For TCP/IP sockets, we want to specify the IP address family (AF_INET) 
and virtual circuit service (SOCK_STREAM). Since there’s only one form 
of virtual circuit service, there are no variations of the protocol, 
so the last argument, protocol, is zero.
*/
create_socket::create_socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0): \
_domain(domain), _type(type), _protocol(protocol)
{
    if ((_server_fd = socket(_domain, _type, _protocol)) < 0) // (AF_INET, SOCK_STREAM, 0)
    {
        std::cerr << "create_socket constructor: Error creating the server FD..." << std::endl;
        exit(EXIT_FAILURE);
    }
}

#endif