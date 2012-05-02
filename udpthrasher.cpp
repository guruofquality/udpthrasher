#include "task_runner.hpp"
#include "udp_common.hpp"
#include <sstream>
#include <fstream>
#include <iostream>

int main(void)
{
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    TaskResult config;
    config.msg = "foo";
    oa << config;
    std::cout << ss.str() << std::endl;
    return 0;
}
