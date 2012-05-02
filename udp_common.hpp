#ifndef UDPTHRASHER_UDP_COMMON_HPP
#define UDPTHRASHER_UDP_COMMON_HPP

#include <boost/asio.hpp>

static const long timeout_us = 100*1000; //100ms

static bool wait_for_recv_ready(int sock_fd){
    //setup timeval for timeout
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout_us;

    //setup rset for timeout
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(sock_fd, &rset);

    //call select with timeout on receive socket
    return ::select(sock_fd+1, &rset, NULL, NULL, &tv) > 0;
}

struct UDPSender{

    static UDPSender *make_overlapped(const std::string &addr, const std::string &port);

    static UDPSender *make_berkeley(const std::string &addr, const std::string &port);

    virtual size_t send(const void *buff, const size_t len) = 0;

};

struct UDPReceiver{

    static UDPReceiver *make_overlapped(const std::string &addr, const std::string &port);

    static UDPReceiver *make_berkeley(const std::string &addr, const std::string &port);

    virtual size_t recv(void *buff, const size_t len) = 0;

};

#endif //UDPTHRASHER_UDP_COMMON_HPP
