#ifndef CREATE_EPOLL_HPP
#define CREATE_EPOLL_HPP
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#define BUFFER_SIZE 1048000
#define EVENTS_SIZE 10
#include "create_socket.hpp"
class create_epoll: public create_socket
{
    private:
        bool _run;
        size_t _ret;
        int _n_events;
        int _epoll_fd;
        struct epoll_event _event, _events[EVENTS_SIZE + 1];
        unsigned char _buffer[BUFFER_SIZE + 1];
        std::vector<create_socket*> sockets;
        std::vector<create_socket*>::iterator it;
    public:
        create_epoll() : _run(1), _n_events(0), _epoll_fd(0), _ret(0)
        {
            if ((_epoll_fd = epoll_create(1)) == -1)
            {
                std::cerr << "Failed to create epoll fd..." << std::endl;
                return ;
            }
            _event._events = EPOLLIN;
            _event.data.fd = 0;
            if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, 0, &event))
            {
                std::cerr << "Failed to add fd to epoll..." << std::endl;
                return ;
            }
            while (_run)
        }

        ~create_epoll()
        {
            if ((close(_epoll_fd)) == -1)
            {
                std::cerr << "Failed to close epoll fd..." << std::endl;
                return ;
            }
        }


};

#endif