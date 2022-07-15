#include "socket.hpp"

int ft::sock::connectToNetwork(int sock, struct sockaddr_in address)
{
	return 0;
}
			
ft::sock::sock(int domain, int service, int protocol, int port, size_t interface)
{
	_addr.sin_family = domain;
	_addr.sin_port = htons(port);
	_addr.sin_addr.s_addr = htonl(interface);
	_sock = socket(domain, service, protocol);
	checkError(_sock);
	_cnct = connectToNetwork(_sock, _addr);
	checkError(_cnct);
}
	
void ft::sock::checkError(int toTest)
{
	if (toTest < 0)
	{
		std::cerr << "Failed to connect to the network..." << std::endl;
		exit(1);
	}
}

int ft::sock::getCnct(void) const
{
	return _cnct;
}

int ft::sock::getSock(void) const
{
	return _sock;
}

struct sockaddr_in ft::sock::getAddr(void) const
{
	return _addr;
}