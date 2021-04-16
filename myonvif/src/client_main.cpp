#include <stdio.h>
#include <stdlib.h>

#include "EchoClient.h"
#include <iostream>

static void help(int ,const char* argv[]){
    printf("Usage:\n");
    printf("    %s 192.168.10.181 192.168.10.66 0\n",argv[0]);
}
int main(int argc,const char* argv[])
{
    int controlType = 0;
    int seq = 0;
    if(argc < 4){
        help(argc,argv);
        return 0;
    }
    EchoClient c;
    c.connect(argv[1],10250);

    std::string ip = argv[2];
    controlType = atoi(argv[3]);
    boost::property_tree::ptree pt;
    //pt.put("topic", "ptz-crtl");
    pt.put("ip", ip);
    pt.put("userName", "admin");
    pt.put("pwd", "hidoo123");
    pt.put("controlType", controlType);
    if(argc > 4){
        seq = atoi(argv[4]);
    }
    pt.put("seq", seq);
    c.request("ptz-crtl",pt);
//    if(c.sendResqust(pt) == 0)
//        c.waitResponse();
    return 0;
}
