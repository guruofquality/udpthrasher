#include "task_runner.hpp"
#include "udp_common.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <csignal>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/thread/thread.hpp>
#include <boost/program_options.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;

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
void server_runner(const std::string &addr, const std::string &port)
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

/***********************************************************************
 * main
 **********************************************************************/
int main(int argc, char *argv[])
{
    std::string addr, port, mode;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("addr", po::value<std::string>(&addr)->default_value("0.0.0.0"), "addr")
        ("port", po::value<std::string>(&port)->default_value("12345"), "port")
        ("mode", po::value<std::string>(&mode)->default_value("server"), "server or client")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("UDP Thrasher %s") % desc << std::endl;
        return ~0;
    }

    if (mode == "server") server_runner(addr, port);

    if (mode == "client")
    {
        asio::io_service io_service;
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), addr, port);
        asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        asio::ip::tcp::socket *socket = new asio::ip::tcp::socket(io_service);
        socket->connect(endpoint);

        TaskRequest req;
        req.num_bytes = 0;
        req.duration = 5.0;
        req.which_impl = "berkeley";
        req.direction = "send";
        req.config.addr = addr;
        req.config.frame_size = 1472;
        req.config.num_frames = 32;
        req.config.sock_buff_size = 0;
        {
            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << req;
            socket->send(asio::buffer(ss.str()));
        }
        /*{
            req.direction = "recv";
            req.addr = "0.0.0.0";
            TaskResult res = run_the_dang_thing(req);
        }*/
        TaskResult res;
        {
            char buff[2048];
            const size_t len = socket->read_some(asio::buffer(buff, sizeof(buff)));
            std::stringstream ss;
            ss << std::string(buff, len);
            boost::archive::text_iarchive ia(ss);
            ia >> res;
        }

        delete socket;

        std::cout << "res.msg " << res.msg << std::endl;
        std::cout << "res.duration " << res.duration << std::endl;
        std::cout << "res.num_bytes " << res.num_bytes << std::endl;
        std::cout << "rate " << (res.num_bytes/res.duration/1e6) << " MBps" << std::endl;

    }

    return 0;
}
