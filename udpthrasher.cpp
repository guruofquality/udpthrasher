#include "task_runner.hpp"
#include "udp_common.hpp"
#include "thrash_server.hpp"
#include "thrash_client.hpp"
#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

namespace po = boost::program_options;

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
        boost::shared_ptr<thrash_client> client(thrash_client::make(addr, port));

        TaskRequest req = TaskRequest();
        req.num_bytes = 0;
        req.duration = 5.0;
        req.which_impl = "berkeley";
        req.direction = "send";
        req.config.addr = "";
        req.config.port = port;
        req.config.frame_size = 1472;
        req.config.num_frames = 32;
        req.config.sock_buff_size = 0;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        client->dispatch_task(req);
        boost::this_thread::sleep(boost::posix_time::seconds(2));

        req.num_bytes = 0;
        req.duration = 5.0;
        req.which_impl = "berkeley";
        req.direction = "recv";
        req.config.addr = "";
        req.config.port = port;
        req.config.frame_size = 1472;
        req.config.num_frames = 32;
        req.config.sock_buff_size = 0;
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        client->dispatch_task(req);

    }

    return 0;
}
