#include "thrash_client.hpp"
#include "task_runner.hpp"
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <iostream>

namespace asio = boost::asio;

void print_res(const TaskResult &res)
{
    if (!res.success)
    {
        std::cout << std::endl << "!!!Failure: " << res.msg << std::endl;
        return;
    }
    std::cout << (res.num_bytes/res.duration/1e6) << " MBps" << std::flush;
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

    void dispatch_rx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration)
    {
        TaskResult res_server = TaskResult();
        TaskResult res_client = TaskResult();
        TaskRequest req_server = TaskRequest();
        TaskRequest req_client = TaskRequest();

        req_client.which_impl = client.which_impl;
        req_client.direction = "recv";
        req_client.num_bytes = num_bytes;
        req_client.duration = duration;
        req_client.config.addr = local_addr;
        req_client.config.port = "45678";
        req_client.config.num_frames = client.num_frames;
        req_client.config.frame_size = client.frame_size;
        req_client.config.sock_buff_size = client.sock_buff_size;

        req_server.which_impl = server.which_impl;
        req_server.direction = "send";
        req_server.num_bytes = num_bytes;
        req_server.duration = duration;
        req_server.config.addr = local_addr;
        req_server.config.port = "45678";
        req_server.config.num_frames = server.num_frames;
        req_server.config.frame_size = server.frame_size;
        req_server.config.sock_buff_size = server.sock_buff_size;

        dispatch_task(req_client, req_server, res_client, res_server);
    }

    void dispatch_tx_task(const TestGoblin &client, const TestGoblin &server, const size_t num_bytes, const double duration)
    {
        TaskResult res_server = TaskResult();
        TaskResult res_client = TaskResult();
        TaskRequest req_server = TaskRequest();
        TaskRequest req_client = TaskRequest();

        req_client.which_impl = client.which_impl;
        req_client.direction = "send";
        req_client.num_bytes = num_bytes;
        req_client.duration = duration;
        req_client.config.addr = remote_addr;
        req_client.config.port = "45678";
        req_client.config.num_frames = client.num_frames;
        req_client.config.frame_size = client.frame_size;
        req_client.config.sock_buff_size = client.sock_buff_size;

        req_server.which_impl = server.which_impl;
        req_server.direction = "recv";
        req_server.num_bytes = num_bytes;
        req_server.duration = duration;
        req_server.config.addr = remote_addr;
        req_server.config.port = "45678";
        req_server.config.num_frames = server.num_frames;
        req_server.config.frame_size = server.frame_size;
        req_server.config.sock_buff_size = server.sock_buff_size;

        dispatch_task(req_client, req_server, res_client, res_server);
    }

    void dispatch_task(const TaskRequest &req_client, const TaskRequest &req_server, TaskResult &res_client, TaskResult &res_server)
    {
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

        std::cout << "    * ";
        std::cout << "Client: " << std::flush;
        print_res(res_client);
        std::cout << std::endl;
        std::cout << "    * ";
        std::cout << "Server: " << std::flush;
        print_res(res_server);
        std::cout << std::endl;

    }

private:
    asio::io_service io_service;
    asio::ip::tcp::socket *socket;
    std::string local_addr, remote_addr;
};

boost::shared_ptr<thrash_client> thrash_client::make(const std::string &addr, const std::string &port)
{
    return boost::make_shared<thrash_client_impl>(addr, port);
}
