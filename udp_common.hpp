#ifndef UDPTHRASHER_UDP_COMMON_HPP
#define UDPTHRASHER_UDP_COMMON_HPP

#include <string>
#include <cstdlib>

struct UDPReceiver{

    static UDPReceiver *make_overlapped(const std::string &addr, const std::string &port, const size_t mtu);

    static UDPReceiver *make_berkeley(const std::string &addr, const std::string &port, const size_t mtu);

    virtual const void *get_buff(size_t &len) = 0;

    virtual void release(void) = 0;

};

struct UDPSender{

    static UDPSender *make_overlapped(const std::string &addr, const std::string &port, const size_t mtu);

    static UDPSender *make_berkeley(const std::string &addr, const std::string &port, const size_t mtu);

    virtual void *get_buff(void) = 0;

    virtual void release(const size_t len) = 0;

};

#endif //UDPTHRASHER_UDP_COMMON_HPP
