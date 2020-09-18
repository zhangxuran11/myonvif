#include <stdio.h>
#include <stdlib.h>

#include "EchoClient.h"
#include <iostream>

int main()
{

    boost::asio::io_context io_context;
    EchoClient c(io_context,"127.0.0.1", 10250);

    int controlType = 0;

    boost::property_tree::ptree pt;
    pt.put("topic", "ptz-crtl");
    pt.put("ip", "192.168.10.66");
    pt.put("userName", "admin");
    pt.put("pwd", "hidoo123");

    while(controlType < 7){
        pt.put("controlType", controlType);
        std::cout<<"controlType="<<controlType<<std::endl;
        c.sendResqust(pt);
        sleep(1);
        controlType++;
    }

    std::cout<<"__cplusplus="<<__cplusplus<<",ATOMIC_INT_LOCK_FREE="<<ATOMIC_INT_LOCK_FREE<<std::endl;

    //io_context.run();
    return 0;
}
