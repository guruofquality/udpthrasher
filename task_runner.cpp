#include "task_runner.hpp"
#include "udp_common.hpp"
#include "high_res_timer.h"
#include <iostream>

using namespace gruel;

static TaskResult run_the_dang_thing_recv(const TaskRequest &req)
{
    UDPReceiver *recver = NULL;
    if (req.which_impl == "overlapped") recver = UDPReceiver::make_overlapped(req.config);
    if (req.which_impl == "berkeley") recver = UDPReceiver::make_berkeley(req.config);
    if (recver == NULL)
    {
        TaskResult res;
        res.success = false;
        res.msg = std::string("unknown impl: ") + req.which_impl;
        return res;
    }

    const high_res_timer_type start_time = high_res_timer_now();
    const high_res_timer_type exit_time = start_time + high_res_timer_type(high_res_timer_tps()*req.duration);

    size_t num_bytes_total = 0;
    while (true)
    {
        size_t len;
        const void *p = recver->get_buff(100/*ms*/, len);
        if (p == NULL)
        {
            delete recver;
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

    delete recver;

    TaskResult res;
    res.success = true;
    res.num_bytes = num_bytes_total;
    res.duration = double(stop_time - start_time)/high_res_timer_tps();
    return res;
}

static TaskResult run_the_dang_thing_send(const TaskRequest &req)
{

    UDPSender *sender = NULL;
    if (req.which_impl == "overlapped") sender = UDPSender::make_overlapped(req.config);
    if (req.which_impl == "berkeley") sender = UDPSender::make_berkeley(req.config);
    if (sender == NULL)
    {
        TaskResult res;
        res.success = false;
        res.msg = std::string("unknown impl: ") + req.which_impl;
        return res;
    }

    const high_res_timer_type start_time = high_res_timer_now();
    const high_res_timer_type exit_time = start_time + high_res_timer_type(high_res_timer_tps()*req.duration);

    size_t num_bytes_total = 0;
    while (true)
    {
        const size_t len = req.config.frame_size;
        void *p = sender->get_buff(100/*ms*/);
        if (p == NULL)
        {
            delete sender;
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

    delete sender;

    TaskResult res;
    res.success = true;
    res.num_bytes = num_bytes_total;
    res.duration = double(stop_time - start_time)/high_res_timer_tps();
    return res;
}

TaskResult run_the_dang_thing(const TaskRequest &req)
{
    if (req.direction == "recv") return run_the_dang_thing_recv(req);
    if (req.direction == "send") return run_the_dang_thing_send(req);
    TaskResult res;
    res.success = false;
    res.msg = std::string("unknown direction: ") + req.direction;
    return res;
}
