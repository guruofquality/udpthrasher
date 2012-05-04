#ifndef UDPTHRASHER_TASK_RUNNER_HPP
#define UDPTHRASHER_TASK_RUNNER_HPP

#include <string>
#include <cstdlib>
#include "udp_common.hpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

struct TaskRequest
{
    std::string which_impl; //berkeley or overlapped
    std::string direction; //recv or send
    UDPSockConfig config;
    size_t num_bytes; //num bytes to run for or 0 for duration
    double duration; //duration in seconds or 0 for num bytes
    double overhead; //simulate overhead with sleep in seconds

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & which_impl;
        ar & direction;
        ar & config;
        ar & num_bytes;
        ar & duration;
        ar & overhead;
    }

};

struct TaskResult
{
    bool success;
    std::string msg;
    size_t num_bytes; //how many bytes transacted
    double duration; //duration in seconds

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int)
    {
        ar & success;
        ar & msg;
        ar & num_bytes;
        ar & duration;
    }

};

TaskResult run_the_dang_thing(const TaskRequest &req);

#endif //UDPTHRASHER_TASK_RUNNER_HPP
