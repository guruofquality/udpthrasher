#include "thrash_server.hpp"
#include "thrash_client.hpp"
#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <vector>

namespace po = boost::program_options;

/***********************************************************************
 * tests to perform
 **********************************************************************/
static void do_tests(thrash_client &tasker, const size_t num_bytes, const double duration)
{
    std::vector<size_t> rx_sock_buff_sizes = boost::assign::list_of
        (0)(10e3)(10e6)(50e6)(100e6);
    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Berkley sockets (client RX)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const size_t rx_sock_buff_size, rx_sock_buff_sizes)
    {
        TestGoblin client = TestGoblin();
        client.which_impl = "berkeley";
        client.sock_buff_size = rx_sock_buff_size;
        TestGoblin server = TestGoblin();
        server.which_impl = "best";
        server.sock_buff_size = 1e6;
        std::cout << boost::format("Client is RXing -- %d rx_sock_buff_size") % rx_sock_buff_size << std::endl;
        tasker.dispatch_rx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
    }

    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Berkley sockets (client TX)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const size_t rx_sock_buff_size, rx_sock_buff_sizes)
    {
        TestGoblin client = TestGoblin();
        client.which_impl = "berkeley";
        client.sock_buff_size = 1e6;
        TestGoblin server = TestGoblin();
        server.sock_buff_size = rx_sock_buff_size;
        std::cout << boost::format("Client is TXing -- %d rx_sock_buff_size") % rx_sock_buff_size << std::endl;
        tasker.dispatch_tx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
    }

    std::vector<size_t> rx_num_frameses = boost::assign::list_of
        (4)(16)(32)(64)(128)(256);
    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Overlapped sockets (client RX)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const size_t rx_num_frames, rx_num_frameses)
    {
        TestGoblin client = TestGoblin();
        client.which_impl = "overlapped";
        client.num_frames = rx_num_frames;
        TestGoblin server = TestGoblin();
        std::cout << boost::format("Client is RXing -- %d rx_num_frames") % rx_num_frames << std::endl;
        tasker.dispatch_rx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
    }

    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Overlapped sockets (client TX)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const size_t rx_num_frames, rx_num_frameses)
    {
        TestGoblin client = TestGoblin();
        client.which_impl = "overlapped";
        TestGoblin server = TestGoblin();
        server.num_frames = rx_num_frames;
        std::cout << boost::format("Client is TXing -- %d rx_num_frames") % rx_num_frames << std::endl;
        tasker.dispatch_tx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
    }

    std::vector<double> overheads = boost::assign::list_of
        (2e-6)(4e-6)(8e-6)(12e-6)(16e-6);
    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Overhead (Berkley sockets)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const double overhead, overheads)
    {
        TestGoblin client = TestGoblin();
        client.sock_buff_size = 50e6;
        client.overhead = overhead;
        client.which_impl = "berkeley";
        TestGoblin server = TestGoblin();
        server.sock_buff_size = 1e6;
        std::cout << boost::format("Client is RXing -- %fus overhead") % (overhead*1e6) << std::endl;
        tasker.dispatch_rx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
        std::cout << boost::format("Client is TXing -- %fus overhead") % (overhead*1e6) << std::endl;
        tasker.dispatch_rx_task(server, client, num_bytes, duration);
        std::cout << std::endl;
    }

    std::cout << "######################################################" << std::endl;
    std::cout << "## testing Overhead (Overlapped sockets)... " << std::endl;
    std::cout << "######################################################" << std::endl;
    BOOST_FOREACH(const double overhead, overheads)
    {
        TestGoblin client = TestGoblin();
        client.overhead = overhead;
        client.which_impl = "overlapped";
        TestGoblin server = TestGoblin();
        std::cout << boost::format("Client is RXing -- %fus overhead") % (overhead*1e6) << std::endl;
        tasker.dispatch_rx_task(client, server, num_bytes, duration);
        std::cout << std::endl;
        std::cout << boost::format("Client is TXing -- %fus overhead") % (overhead*1e6) << std::endl;
        tasker.dispatch_rx_task(server, client, num_bytes, duration);
        std::cout << std::endl;
    }
}

/***********************************************************************
 * main
 **********************************************************************/
int main(int argc, char *argv[])
{
    std::string addr, port, mode;
    size_t num_bytes;
    double duration;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("addr", po::value<std::string>(&addr)->default_value("0.0.0.0"), "addr")
        ("port", po::value<std::string>(&port)->default_value("12345"), "port")
        ("num_bytes", po::value<size_t>(&num_bytes)->default_value(0), "num bytes to xfer or 0 for duration")
        ("duration", po::value<double>(&duration)->default_value(0.0), "duration in seconds or 0 for num_bytes")
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

    if (mode == "server"){
        thrash_server(addr, port);
    }

    if (mode == "client")
    {
        boost::shared_ptr<thrash_client> client(thrash_client::make(addr, port));
        do_tests(*client, num_bytes, duration);
    }

    return 0;
}
