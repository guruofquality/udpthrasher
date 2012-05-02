#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <stdexcept>
#include <vector>

namespace asio = boost::asio;

//TODO param me...
static const size_t num_recv_frames = 32;
static const size_t num_send_frames = 32;

#ifdef _MSC_VER

/***********************************************************************
 * Static initialization to take care of WSA init and cleanup
 **********************************************************************/
struct wsa_control{
    wsa_control(void){
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData); /*windows socket startup */
    }

    ~wsa_control(void){
        WSACleanup();
    }
};


/***********************************************************************
 * Overlapped Receive Implementation
 **********************************************************************/
class UDPReceiverOverlapped : public UDPReceiver{
public:

    UDPReceiverOverlapped(const std::string &addr, const std::string &port, const size_t mtu)
    {
        static wsa_control wsa; //makes wsa start happen via lazy initialization

        asio::io_service _io_service;
        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        //create the socket
        _sock_fd = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_sock_fd == INVALID_SOCKET){
            const DWORD error = WSAGetLastError();
            throw std::runtime_error(str(boost::format("WSASocket() failed with error %d") % error));
        }

        //bind the socket
        const asio::ip::udp::endpoint::data_type &servaddr = *endpoint.data();
        if (::bind(_sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
            const DWORD error = WSAGetLastError();
            closesocket(_sock_fd);
            throw std::runtime_error(str(boost::format("bind() failed with error %d") % error));
        }
    }

    ~UDPReceiverOverlapped(void)
    {
        closesocket(_sock_fd);
    }

    const void *get_buff(size_t &len)
    {
        
    }

    void release(void)
    {
        
    }

private:
    int _sock_fd;
    std::vector<WSAOVERLAPPED> _overlapped;
};

/***********************************************************************
 * Overlapped Send Implementation
 **********************************************************************/
class UDPSenderOverlapped : public UDPSender{
public:
    UDPSenderOverlapped(const std::string &addr, const std::string &port, const size_t mtu)
    {
        static wsa_control wsa; //makes wsa start happen via lazy initialization

        asio::io_service _io_service;
        asio::ip::udp::resolver resolver(_io_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, port);
        asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

        //create the socket
        _sock_fd = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_sock_fd == INVALID_SOCKET){
            const DWORD error = WSAGetLastError();
            throw std::runtime_error(str(boost::format("WSASocket() failed with error %d") % error));
        }

        //connect the socket so we can send/recv
        const asio::ip::udp::endpoint::data_type &servaddr = *endpoint.data();
        if (WSAConnect(_sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr), NULL, NULL, NULL, NULL) != 0){
            const DWORD error = WSAGetLastError();
            closesocket(_sock_fd);
            throw std::runtime_error(str(boost::format("WSAConnect() failed with error %d") % error));
        }
    }

    ~UDPSenderOverlapped(void)
    {
        closesocket(_sock_fd);
    }

    void *get_buff(void)
    {
        
    }

    void release(const size_t len)
    {
        
    }

private:
    int _sock_fd;
};

/***********************************************************************
 * Factory functions
 **********************************************************************/
UDPReceiver *UDPReceiver::make_overlapped(const std::string &addr, const std::string &port, const size_t mtu)
{
    return new UDPReceiverOverlapped(addr, port, mtu);
}

UDPSender *UDPSender::make_overlapped(const std::string &addr, const std::string &port, const size_t mtu)
{
    return new UDPSenderOverlapped(addr, port, mtu);
}

#else

UDPReceiver *UDPReceiver::make_overlapped(const std::string &, const std::string &, const size_t)
{
    throw std::runtime_error("cannot make_overlapped, this is not windows");
}

UDPSender *UDPSender::make_overlapped(const std::string &, const std::string &, const size_t)
{
    throw std::runtime_error("cannot make_overlapped, this is not windows");
}

#endif
