#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <stdexcept>

#ifdef _MSC_VER

class UDPSenderOverlapped : public UDPSender{
public:
};

class UDPReceiverOverlapped : public UDPReceiver{
public:
};

UDPReceiver *UDPReceiver::make_overlapped(const std::string &addr, const std::string &port)
{
    return new UDPReceiverOverlapped(addr, port);
}

UDPSender *UDPSender::make_overlapped(const std::string &addr, const std::string &port)
{
    return new UDPSenderOverlapped(addr, port);
}

#else

UDPReceiver *UDPReceiver::make_overlapped(const std::string &addr, const std::string &port)
{
    throw std::runtime_error("cannot make_overlapped, this is not windows");
}

UDPSender *UDPSender::make_overlapped(const std::string &addr, const std::string &port)
{
    throw std::runtime_error("cannot make_overlapped, this is not windows");
}

#endif
