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

    UDPReceiverOverlapped(const std::string &addr, const std::string &port, const size_t mtu):
        _index(0)
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

        //init buffers
        _xfers.resize(num_recv_frames);
        for (size_t i = 0; i < _xfers.size(); i++)
        {
            xfer_struct &xfer = _xfers[i];
            xfer.mem.resize(mtu);
            xfer.buf.buf = &xfer.mem[0];
            xfer.buf.len = xfer.mem.size();
            ZeroMemory(&xfer.overlapped, sizeof(xfer.overlapped));
            xfer.overlapped.hEvent = WSACreateEvent();
            if (xfer.overlapped.hEvent == WSA_INVALID_EVENT){
                throw std::runtime_error("xfer.overlapped.hEvent == WSA_INVALID_EVENT");
            }
            this->release();
        }

    }

    ~UDPReceiverOverlapped(void)
    {
        for (size_t i = 0; i < _xfers.size(); i++)
        {
            WSACloseEvent(_xfers[i].overlapped.hEvent);
        }
        closesocket(_sock_fd);
    }

    const void *get_buff(const size_t timeout_ms, size_t &len)
    {
        xfer_struct &xfer = _xfers[_index];

        const DWORD result = WSAWaitForMultipleEvents(1, &xfer.overlapped.hEvent, true, timeout_ms, true);
        if (result == WSA_WAIT_TIMEOUT) return NULL;

        DWORD flags = 0;
        WSAGetOverlappedResult(_sock_fd, &xfer.overlapped, &xfer.bytes, true, &flags);
        WSAResetEvent(xfer.overlapped.hEvent);

        len = xfer.bytes;
        return xfer.buf.buf;
    }

    void release(void)
    {
        xfer_struct &xfer = _xfers[_index];
        DWORD flags = 0;
        WSARecv(_sock_fd, &xfer.buf, 1, &xfer.bytes, &flags, &xfer.overlapped, NULL);

        if (++_index == _xfers.size()) _index = 0;
    }

private:
    int _sock_fd;
    struct xfer_struct{
        WSAOVERLAPPED overlapped;
        WSABUF buf;
        DWORD bytes;
        std::vector<char> mem;
    };
    std::vector<xfer_struct> _xfers;
    size_t _index;
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

        //init buffers
        _xfers.resize(num_recv_frames);
        for (size_t i = 0; i < _xfers.size(); i++)
        {
            xfer_struct &xfer = _xfers[i];
            xfer.mem.resize(mtu);
            xfer.buf.buf = &xfer.mem[0];
            xfer.buf.len = xfer.mem.size();
            ZeroMemory(&xfer.overlapped, sizeof(xfer.overlapped));
            xfer.overlapped.hEvent = WSACreateEvent();
            if (xfer.overlapped.hEvent == WSA_INVALID_EVENT){
                throw std::runtime_error("xfer.overlapped.hEvent == WSA_INVALID_EVENT");
            }
            WSASetEvent(xfer.overlapped.hEvent); //makes buffer available via get
        }

    }

    ~UDPSenderOverlapped(void)
    {
        for (size_t i = 0; i < _xfers.size(); i++)
        {
            WSACloseEvent(_xfers[i].overlapped.hEvent);
        }
        closesocket(_sock_fd);
    }

    void *get_buff(const size_t timeout_ms)
    {
        xfer_struct &xfer = _xfers[_index];

        const DWORD result = WSAWaitForMultipleEvents(1, &xfer.overlapped.hEvent, true, timeout_ms, true);
        if (result == WSA_WAIT_TIMEOUT) return NULL;

        WSAResetEvent(xfer.overlapped.hEvent);
        return xfer.buf.buf;
    }

    void release(const size_t len)
    {
        xfer_struct &xfer = _xfers[_index];

        xfer.buf.len = len;
        WSASend(_sock_fd, &xfer.buf, 1, NULL, 0, &xfer.overlapped, NULL);

        if (++_index == _xfers.size()) _index = 0;
    }

private:
    int _sock_fd;
    struct xfer_struct{
        WSAOVERLAPPED overlapped;
        WSABUF buf;
        DWORD bytes;
        std::vector<char> mem;
    };
    std::vector<xfer_struct> _xfers;
    size_t _index;
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
