#ifndef UDPTHRASHER_TRASH_CLIENT_HPP
#define UDPTHRASHER_TRASH_CLIENT_HPP

#include <string>
#include <boost/shared_ptr.hpp>

struct TestGoblin
{
    TestGoblin(void)
    {
        which_impl = "best";
        sock_buff_size = 0;
        frame_size = 1472;
        num_frames = 32;
        overhead = 0.0;
    }
    std::string which_impl;
    size_t sock_buff_size;
    size_t frame_size;
    size_t num_frames;
    double overhead;
};

struct thrash_client
{
    static boost::shared_ptr<thrash_client> make(const std::string &addr, const std::string &port);

    virtual void dispatch_rx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration) = 0;

    virtual void dispatch_tx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration) = 0;
};

#endif //UDPTHRASHER_TRASH_CLIENT_HPP
