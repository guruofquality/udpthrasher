#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <vector>

namespace asio = boost::asio;

static inline bool wait_for_recv_ready(int sock_fd, const size_t timeout_ms){
    //setup timeval for timeout
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms*1000;

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

    UDPReceiverBerkeley(const UDPSockConfig &config)
    {
        _buff.resize(config.frame_size);

        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), config.addr, config.port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        _socket = new asio::ip::udp::socket(_io_service, endpoint);

        if (config.sock_buff_size > 0) setsockopt(
            _socket->native(), SOL_SOCKET, SO_RCVBUF,
            (const char *)&config.sock_buff_size,
            sizeof(config.sock_buff_size)
        );
    }

    ~UDPReceiverBerkeley(void)
    {
        delete _socket;
    }

    const void *get_buff(const size_t timeout_ms, size_t &len)
    {
        if (!wait_for_recv_ready(_socket->native(), timeout_ms))
        {
            return NULL;
        }
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

    UDPSenderBerkeley(const UDPSockConfig &config)
    {
        _buff.resize(config.frame_size);

        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), config.addr, config.port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        _socket = new asio::ip::udp::socket(_io_service);
        _socket->open(asio::ip::udp::v4());
        _socket->connect(endpoint);

        if (config.sock_buff_size > 0) setsockopt(
            _socket->native(), SOL_SOCKET, SO_SNDBUF,
            (const char *)&config.sock_buff_size,
            sizeof(config.sock_buff_size)
        );

    }

    ~UDPSenderBerkeley(void)
    {
        delete _socket;
    }

    void *get_buff(const size_t)
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
UDPReceiver *UDPReceiver::make_berkeley(const UDPSockConfig &config)
{
    return new UDPReceiverBerkeley(config);
}

UDPSender *UDPSender::make_berkeley(const UDPSockConfig &config)
{
    return new UDPSenderBerkeley(config);
}
