#include "thrash_client.hpp"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>

namespace asio = boost::asio;

void print_res(TaskResult &res)
{
    std::cout << "res.msg " << res.msg << std::endl;
    std::cout << "res.duration " << res.duration << std::endl;
    std::cout << "res.num_bytes " << res.num_bytes << std::endl;
    std::cout << "rate " << (res.num_bytes/res.duration/1e6) << " MBps" << std::endl;
}

class thrash_client_impl : public thrash_client
{
public:
    thrash_client_impl(const std::string &addr, const std::string &port)
    {
        asio::ip::tcp::resolver resolver(io_service);
        asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), addr, port);
        asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        socket = new asio::ip::tcp::socket(io_service);
        socket->connect(endpoint);

        local_addr = socket->local_endpoint().address().to_string();
        remote_addr = socket->remote_endpoint().address().to_string();

        std::cout << "local_addr " << local_addr << std::endl;
        std::cout << "remote_addr " << remote_addr << std::endl;
    }

    ~thrash_client_impl(void)
    {
        delete socket;
    }

    void dispatch_task(TaskRequest &req)
    {
        TaskResult res_server = TaskResult();
        TaskResult res_client = TaskResult();

        //setup client side task
        TaskRequest req_server = req;
        TaskRequest req_client = req;
        if (req.direction == "send") req_client.direction = "recv";
        if (req.direction == "recv") req_client.direction = "send";

        if (req_server.direction == "recv") req_server.config.addr = remote_addr;
        if (req_server.direction == "send") req_server.config.addr = local_addr;

        if (req_client.direction == "recv") req_client.config.addr = local_addr;
        if (req_client.direction == "send") req_client.config.addr = remote_addr;

        if (req_server.direction == "recv") req_server.config.sock_buff_size = 50e6;
        if (req_client.direction == "recv") req_client.config.sock_buff_size = 50e6;
        if (req_server.direction == "send") req_server.config.sock_buff_size = 1e6;
        if (req_client.direction == "send") req_client.config.sock_buff_size = 1e6;

        //tell server to do it
        {
            std::stringstream ss;
            boost::archive::text_oarchive oa(ss);
            oa << req_server;
            socket->send(asio::buffer(ss.str()));
        }

        //get result from client
        res_client = run_the_dang_thing(req_client);

        //get result from server
        {
            char buff[2048];
            const size_t len = socket->receive(asio::buffer(buff, sizeof(buff)));
            std::stringstream ss;
            ss << std::string(buff, len);
            boost::archive::text_iarchive ia(ss);
            ia >> res_server;
        }

        std::cout << "Client result:" << std::endl;
        print_res(res_client);
        std::cout << "Server result:" << std::endl;
        print_res(res_server);

    }

private:
    asio::io_service io_service;
    asio::ip::tcp::socket *socket;
    std::string local_addr, remote_addr;
};

thrash_client *thrash_client::make(const std::string &addr, const std::string &port)
{
    return new thrash_client_impl(addr, port);
}
