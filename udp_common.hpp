#ifndef UDPTHRASHER_UDP_COMMON_HPP
#define UDPTHRASHER_UDP_COMMON_HPP

#include <string>
#include <cstdlib>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/asio.hpp> //select
#include <boost/shared_ptr.hpp>

static inline bool wait_for_recv_ready(int sock_fd, const size_t timeout_ms)
{
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

struct UDPSockConfig
{
    std::string addr;
    std::string port;
    size_t frame_size;
    size_t num_frames;
    int sock_buff_size;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & addr;
        ar & port;
        ar & frame_size;
        ar & num_frames;
        ar & sock_buff_size;
    }

};

struct UDPReceiver
{

    static boost::shared_ptr<UDPReceiver> make_overlapped(const UDPSockConfig &config);

    static boost::shared_ptr<UDPReceiver> make_berkeley(const UDPSockConfig &config);

    virtual const void *get_buff(const size_t timeout_ms, size_t &len) = 0;

    virtual void release(void) = 0;

};

struct UDPSender
{

    static boost::shared_ptr<UDPSender> make_overlapped(const UDPSockConfig &config);

    static boost::shared_ptr<UDPSender> make_berkeley(const UDPSockConfig &config);

    virtual void *get_buff(const size_t timeout_ms) = 0;

    virtual void release(const size_t len) = 0;

};

#endif //UDPTHRASHER_UDP_COMMON_HPP
