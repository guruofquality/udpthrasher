#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <boost/make_shared.hpp>
#include <iostream>

namespace asio = boost::asio;

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

        _socket = boost::shared_ptr<asio::ip::udp::socket>(new asio::ip::udp::socket(_io_service, endpoint));

        if (config.sock_buff_size > 0)
        {
            asio::socket_base::receive_buffer_size option(config.sock_buff_size);
            _socket->set_option(option);
        }
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
    boost::shared_ptr<asio::ip::udp::socket> _socket;
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

        _socket = boost::shared_ptr<asio::ip::udp::socket>(new asio::ip::udp::socket(_io_service));
        _socket->open(asio::ip::udp::v4());
        _socket->connect(endpoint);

        if (config.sock_buff_size > 0)
        {
            asio::socket_base::send_buffer_size option(config.sock_buff_size);
            _socket->set_option(option);
        }

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
    boost::shared_ptr<asio::ip::udp::socket> _socket;
    std::vector<char> _buff;
};

/***********************************************************************
 * Factory functions
 **********************************************************************/
boost::shared_ptr<UDPReceiver> UDPReceiver::make_berkeley(const UDPSockConfig &config)
{
    return boost::make_shared<UDPReceiverBerkeley>(config);
}

boost::shared_ptr<UDPSender> UDPSender::make_berkeley(const UDPSockConfig &config)
{
    return boost::make_shared<UDPSenderBerkeley>(config);
}
