#ifndef UDPTHRASHER_TRASH_CLIENT_HPP
#define UDPTHRASHER_TRASH_CLIENT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

struct TestGoblin
{
    std::string which_impl;
    size_t sock_buff_size;
    size_t frame_size;
    size_t num_frames;
};

struct thrash_client
{
    static boost::shared_ptr<thrash_client> make(const std::string &addr, const std::string &port);

    virtual void dispatch_rx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration) = 0;

    virtual void dispatch_tx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration) = 0;
};

#endif //UDPTHRASHER_TRASH_CLIENT_HPP
