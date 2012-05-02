#ifndef UDPTHRASHER_TRASH_CLIENT_HPP
#define UDPTHRASHER_TRASH_CLIENT_HPP

#include <string>
#include "task_runner.hpp"

struct thrash_client
{
    static thrash_client *make(const std::string &addr, const std::string &port);

    virtual void dispatch_task(TaskRequest &req) = 0;
};

#endif //UDPTHRASHER_TRASH_CLIENT_HPP
