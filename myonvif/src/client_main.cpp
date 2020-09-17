#include <stdio.h>
#include <stdlib.h>

#include "EchoClient.h"


int main()
{

    boost::asio::io_context io_context;
    EchoClient c(io_context,"192.168.10.6", 10250);

    boost::property_tree::ptree pt;
    pt.put("cmd", "test");
    pt.put("action", "this is a etst");
    c.sendResqust(pt);

    io_context.run();
    return 0;
}
