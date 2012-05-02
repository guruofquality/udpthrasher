#include "udp_common.hpp"
#include <boost/asio.hpp>

class UDPReceiverBerkeley : public UDPReceiver{
public:

    UDPReceiverBerkeley(const std::string &addr, const std::string &port)
    {
        
    }

    ~UDPReceiverBerkeley(void)
    {
        
    }

    size_t recv(void *buff, const size_t len)
    {
        
    }

};


class UDPSenderBerkeley : public UDPSender{
public:

    UDPSenderBerkeley(const std::string &addr, const std::string &port)
    {
        
    }

    ~UDPSenderBerkeley(void)
    {
        
    }

    size_t send(const void *buff, const size_t len)
    {
        
    }

};

UDPReceiver *UDPReceiver::make_berkeley(const std::string &addr, const std::string &port)
{
    return new UDPReceiverBerkeley(addr, port);
}

UDPSender *UDPSender::make_berkeley(const std::string &addr, const std::string &port)
{
    return new UDPSenderBerkeley(addr, port);
}
