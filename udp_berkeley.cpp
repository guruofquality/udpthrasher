#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <vector>

namespace asio = boost::asio;

static const long timeout_us = 100*1000; //100ms

static inline bool wait_for_recv_ready(int sock_fd){
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

/***********************************************************************
 * Berkeley Receive Implementation
 **********************************************************************/
class UDPReceiverBerkeley : public UDPReceiver{
public:

    UDPReceiverBerkeley(const std::string &addr, const std::string &port, const size_t mtu)
    {
        _buff.resize(mtu);

        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        _socket = new asio::ip::udp::socket(_io_service, endpoint);
    }

    ~UDPReceiverBerkeley(void)
    {
        delete _socket;
    }

    const void *get_buff(size_t &len)
    {
        len = 0; //reset
        if (wait_for_recv_ready(_socket->native()))
            len = ::recv(_socket->native(), &_buff[0], _buff.size(), 0);
        return &_buff[0];
    }

    void release(void)
    {
        //NOP
    }

private:
    asio::io_service _io_service;
    asio::ip::udp::socket *_socket;
    std::vector<char> _buff;
};

/***********************************************************************
 * Berkeley Send Implementation
 **********************************************************************/
class UDPSenderBerkeley : public UDPSender{
public:

    UDPSenderBerkeley(const std::string &addr, const std::string &port, const size_t mtu)
    {
        _buff.resize(mtu);

        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        _socket = new asio::ip::udp::socket(_io_service);
        _socket->open(asio::ip::udp::v4());
        _socket->connect(endpoint);
    }

    ~UDPSenderBerkeley(void)
    {
        delete _socket;
    }

    void *get_buff(void)
    {
        return &_buff[0];
    }

    void release(const size_t len)
    {
        ::send(_socket->native(), &_buff[0], len, 0);
    }

private:
    asio::io_service _io_service;
    asio::ip::udp::socket *_socket;
    std::vector<char> _buff;
};

/***********************************************************************
 * Factory functions
 **********************************************************************/
UDPReceiver *UDPReceiver::make_berkeley(const std::string &addr, const std::string &port, const size_t mtu)
{
    return new UDPReceiverBerkeley(addr, port, mtu);
}

UDPSender *UDPSender::make_berkeley(const std::string &addr, const std::string &port, const size_t mtu)
{
    return new UDPSenderBerkeley(addr, port, mtu);
}
