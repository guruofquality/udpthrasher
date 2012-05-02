#include "thrash_server.hpp"
#include "task_runner.hpp"
#include "udp_common.hpp"
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <csignal>

namespace asio = boost::asio;

/***********************************************************************
 * Signal handlers
 **********************************************************************/
static bool stop_signal_called = false;
void sig_int_handler(int){stop_signal_called = true;}

/***********************************************************************
 * server handler
 **********************************************************************/
void handle(asio::ip::tcp::socket *socket)
{
    std::cout << "started a new handler thread" << std::endl;
    while (!stop_signal_called)
    {
        if (!wait_for_recv_ready(socket->native(), 100)) continue;
        TaskRequest req;
        {
            char buff[2048];
            boost::system::error_code error;
            const size_t len = socket->read_some(asio::buffer(buff, sizeof(buff)), error);
            if (error == boost::asio::error::eof) break;
            std::stringstream ss;
            ss << std::string(buff, len);
            boost::archive::text_iarchive ia(ss);
            ia >> req;
        }
        std::cout << "before the run... " << std::endl;
        TaskResult res = run_the_dang_thing(req);
        std::cout << "after the run... " << std::endl;
        {
            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << res;
            std::string out = ss.str();
            socket->send(asio::buffer(out));
        }
    }
    delete socket;
}

/***********************************************************************
 * server dispatcher
 **********************************************************************/
void thrash_server(const std::string &addr, const std::string &port)
{
    std::signal(SIGINT, &sig_int_handler);
    std::cout << "Press Ctrl + C to stop server..." << std::endl;

    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), addr, port);
    asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    asio::ip::tcp::acceptor *acceptor = new asio::ip::tcp::acceptor(io_service, endpoint);

    boost::thread_group thread_group;

    while (!stop_signal_called)
    {
        if (!wait_for_recv_ready(acceptor->native(), 100)) continue;
        asio::ip::tcp::socket *socket = new asio::ip::tcp::socket(io_service);
        acceptor->accept(*socket);

        thread_group.create_thread(boost::bind(&handle, socket));
    }

    std::cout << "cleanup server runner..." << std::endl;

    delete acceptor;
    thread_group.interrupt_all();
    thread_group.join_all();

    std::cout << "server runner done!" << std::endl;
}
