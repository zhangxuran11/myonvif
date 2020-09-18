#include <stdio.h>
#include <stdlib.h>

#include "OnvifSoap.h"
#include "EchoServer.h"


#define SOAP_SOCK_TIMEOUT    (10)                                               // socket超时时间（单位秒）

#define USERNAME    "admin"
#define PASSWORD    "hidoo123"

void ptzCrtlProcesser(OnvifSoap* soap, const boost::property_tree::ptree& pt){
    std::stringstream ss;
    boost::property_tree::write_json(ss, pt);
    std::cout<<ss.str()<<std::endl;

    std::string ip = pt.get<std::string>("ip");
    std::string userName = pt.get<std::string>("userName");
    std::string pwd = pt.get<std::string>("pwd");

    int controlType = pt.get<int>("controlType");
    soap->continuousMove(ip.c_str(),userName.c_str(),pwd.c_str(),controlType);

}
int main()
{
    OnvifSoap soap(SOAP_SOCK_TIMEOUT);

    boost::asio::io_context io_context;
    EchoServer s(io_context, 10250);
    s.registerProcesser("ptz-crtl",std::bind(ptzCrtlProcesser,&soap,std::placeholders::_1));

    io_context.run();

    return 0;
}
