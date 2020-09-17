#include <stdio.h>
#include <stdlib.h>

#include "OnvifSoap.h"
#include "EchoServer.h"


#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单位秒）

#define USERNAME    "admin"
#define PASSWORD    "hidoo123"
static void cb(char *DeviceXAddr){
    printf("%s\n",DeviceXAddr);
}
void testProcesser(const boost::property_tree::ptree& pt){
    std::stringstream ss;
    boost::property_tree::write_json(ss, pt);
    std::cout<<ss.str()<<std::endl;
}
int main()
{
    OnvifSoap soap(SOAP_SOCK_TIMEOUT);

    boost::asio::io_context io_context;
    EchoServer s(io_context, 10250);
    s.registerProcesser("test",testProcesser);

    io_context.run();


    const char* ip = "192.168.10.66";
    soap.detectDevice(cb);
    soap.continuousMove(ip,USERNAME,PASSWORD);
    return 0;
}
