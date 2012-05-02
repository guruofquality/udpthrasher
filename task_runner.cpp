#include "task_runner.hpp"
#include "udp_common.hpp"
#include "high_res_timer.h"
#include <iostream>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>

using namespace gruel;

static TaskResult run_the_dang_thing_recv(const TaskRequest &req)
{
    boost::shared_ptr<UDPReceiver> recver;
    if (req.which_impl == "overlapped") recver = UDPReceiver::make_overlapped(req.config);
    if (req.which_impl == "berkeley") recver = UDPReceiver::make_berkeley(req.config);
    if (!recver)
    {
        TaskResult res;
        res.success = false;
        res.msg = std::string("unknown impl: ") + req.which_impl;
        return res;
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    const high_res_timer_type start_time = high_res_timer_now();
    const high_res_timer_type exit_time = start_time + high_res_timer_type(high_res_timer_tps()*req.duration);

    size_t num_bytes_total = 0;
    while (true)
    {
        size_t len;
        const void *p = recver->get_buff(100/*ms*/, len);
        if (p == NULL)
        {
            TaskResult res;
            res.success = false;
            res.msg = "timeout";
            return res;
        }
        recver->release();
        num_bytes_total += len;

        if (req.num_bytes != 0 && num_bytes_total >= req.num_bytes)
        {
            break;
        }

        if (req.duration != 0 && high_res_timer_now() >= exit_time)
        {
            break;
        }

    }

    const high_res_timer_type stop_time = high_res_timer_now();
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    TaskResult res;
    res.success = true;
    res.num_bytes = num_bytes_total;
    res.duration = double(stop_time - start_time)/high_res_timer_tps();
    return res;
}

static TaskResult run_the_dang_thing_send(const TaskRequest &req)
{

    boost::shared_ptr<UDPSender> sender;
    if (req.which_impl == "overlapped") sender = UDPSender::make_overlapped(req.config);
    if (req.which_impl == "berkeley") sender = UDPSender::make_berkeley(req.config);
    if (!sender)
    {
        TaskResult res;
        res.success = false;
        res.msg = std::string("unknown impl: ") + req.which_impl;
        return res;
    }

    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    const high_res_timer_type start_time = high_res_timer_now();
    const high_res_timer_type exit_time = start_time + high_res_timer_type(high_res_timer_tps()*req.duration);

    size_t num_bytes_total = 0;
    while (true)
    {
        const size_t len = req.config.frame_size;
        void *p = sender->get_buff(100/*ms*/);
        if (p == NULL)
        {
            TaskResult res;
            res.success = false;
            res.msg = "timeout";
            return res;
        }
        sender->release(len);
        num_bytes_total += len;

        if (req.num_bytes != 0 && num_bytes_total >= req.num_bytes)
        {
            break;
        }

        if (req.duration != 0 && high_res_timer_now() >= exit_time)
        {
            break;
        }
    }

    const high_res_timer_type stop_time = high_res_timer_now();
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    TaskResult res;
    res.success = true;
    res.num_bytes = num_bytes_total;
    res.duration = double(stop_time - start_time)/high_res_timer_tps();
    return res;
}

TaskResult run_the_dang_thing(const TaskRequest &req_)
{
    TaskRequest req = req_;

    if (req.which_impl == "best") req.which_impl = "overlapped";

    //we know overlapped will fail, so use other impl
    #ifndef _MSC_VER
    req.which_impl = "berkeley";
    #endif

    try
    {
        if (req.direction == "recv") return run_the_dang_thing_recv(req);
        if (req.direction == "send") return run_the_dang_thing_send(req);
        TaskResult res;
        res.success = false;
        res.msg = std::string("unknown direction: ") + req.direction;
        return res;
    }
    catch(const std::exception &ex)
    {
        TaskResult res;
        res.success = false;
        res.msg = std::string("error: ") + ex.what();
        return res;
    }
    TaskResult res;
    res.success = false;
    res.msg = std::string("error unknown");
    return res;
}
