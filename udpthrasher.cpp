#include "task_runner.hpp"
#include "udp_common.hpp"
#include "thrash_server.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>

namespace asio = boost::asio;
namespace po = boost::program_options;

void gogo(TaskRequest req)
{
    TaskResult res = run_the_dang_thing(req);
    std::cout << "res.msg " << res.msg << std::endl;
    std::cout << "res.duration " << res.duration << std::endl;
    std::cout << "res.num_bytes " << res.num_bytes << std::endl;
    std::cout << "rate " << (res.num_bytes/res.duration/1e6) << " MBps" << std::endl;
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

    if (mode == "server") thrash_server(addr, port);

    if (mode == "client")
    {
        asio::io_service io_service;
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), addr, port);
        asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        asio::ip::tcp::socket *socket = new asio::ip::tcp::socket(io_service);
        socket->connect(endpoint);

        const std::string local_addr = socket->local_endpoint().address().to_string();
        const std::string remote_addr = socket->remote_endpoint().address().to_string();
        std::cout << "local_addr " << local_addr << std::endl;
        std::cout << "remote_addr " << remote_addr << std::endl;

        boost::thread_group thread_group;
        {
            TaskRequest req;
            req.num_bytes = 0;
            req.duration = 10.0;
            req.which_impl = "berkeley";
            req.direction = "recv";
            req.config.addr = "0.0.0.0";
            req.config.port = port;
            req.config.frame_size = 1472;
            req.config.num_frames = 32;
            req.config.sock_buff_size = 0;
            thread_group.create_thread(boost::bind(&gogo, req));
        }

        TaskRequest req;
        req.num_bytes = 0;
        req.duration = 10.0;
        req.which_impl = "berkeley";
        req.direction = "send";
        req.config.addr = local_addr;
        req.config.port = port;
        req.config.frame_size = 1472;
        req.config.num_frames = 32;
        req.config.sock_buff_size = 0;
        {
            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << req;
            socket->send(asio::buffer(ss.str()));
        }
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
