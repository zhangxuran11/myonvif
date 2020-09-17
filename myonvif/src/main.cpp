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

int main(int argc, char **argv)
{
    boost::asio::io_context io_context;

    server s(io_context, std::atoi(argv[1]));

    io_context.run();

    OnvifSoap soap(SOAP_SOCK_TIMEOUT);
    const char* ip = "192.168.10.66";
    soap.detectDevice(cb);
    //soap.absoluteMove(ip,USERNAME,PASSWORD);
    soap.continuousMove(ip,USERNAME,PASSWORD);
    //ONVIF_PTZ_ContinuousMove(&soap,ip,profiles.c_str(),ptzXAddr.c_str());
    return 0;
}
