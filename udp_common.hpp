#ifndef UDPTHRASHER_UDP_COMMON_HPP
#define UDPTHRASHER_UDP_COMMON_HPP

#include <string>
#include <cstdlib>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

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

    static UDPReceiver *make_overlapped(const UDPSockConfig &config);

    static UDPReceiver *make_berkeley(const UDPSockConfig &config);

    virtual const void *get_buff(const size_t timeout_ms, size_t &len) = 0;

    virtual void release(void) = 0;

};

struct UDPSender
{

    static UDPSender *make_overlapped(const UDPSockConfig &config);

    static UDPSender *make_berkeley(const UDPSockConfig &config);

    virtual void *get_buff(const size_t timeout_ms) = 0;

    virtual void release(const size_t len) = 0;

};

#endif //UDPTHRASHER_UDP_COMMON_HPP
