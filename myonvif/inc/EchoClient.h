#ifndef ECHO_CLIENT_H
#define ECHO_CLIENT_H
#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
class EchoClient{
    public:
        EchoClient():mSocket(mIoContext){
        }
        int connect(const char* serverIp,uint16_t port){
            boost::system::error_code ec;
            mSocket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(serverIp),port),ec);
            if( ec )
                return -1;
            else
                return 0;
        }
        int connect(const char* eurekaIp,uint16_t eurekaPort,const char* serviceName){
            mEurekaIp = eurekaIp;
            mEurekaPort = eurekaPort;
            mServiceName = serviceName;
            return connect();
        }
        
    
        ~EchoClient(){
            mSocket.close();
        }
        boost::property_tree::ptree request(const std::string& topic,const boost::property_tree::ptree& pt){
            std::stringstream ss;
            boost::property_tree::ptree root;
            root.put("topic",topic);
            root.put_child("content",pt);
            boost::property_tree::json_parser::write_json(ss,root);
            boost::system::error_code ec;
            mSendBuffer.resize(ss.str().size()+4);
            *reinterpret_cast<uint32_t*>(mSendBuffer.data()) = ss.str().size();
            memcpy(mSendBuffer.data()+4,ss.str().data(),ss.str().size());
            boost::asio::write(mSocket,boost::asio::buffer(mSendBuffer,mSendBuffer.size()),ec);
            if(ec)
            {
                return boost::property_tree::ptree();
            }
            mRecvBuffer.resize(4);
            boost::asio::read(mSocket,boost::asio::buffer(mRecvBuffer,mRecvBuffer.size()),ec);
            if(ec)
            {
                return boost::property_tree::ptree();
            }
            uint32_t len = *reinterpret_cast<uint32_t*>(mRecvBuffer.data());
            mRecvBuffer.resize(len);
            boost::asio::read(mSocket,boost::asio::buffer(mRecvBuffer,mRecvBuffer.size()),ec);
            if(ec)
            {
                return boost::property_tree::ptree();
            }
            ss.str("");
            ss<<std::string(mRecvBuffer.data(),mRecvBuffer.size());
            boost::property_tree::ptree res;
            boost::property_tree::json_parser::read_json(ss,res);
            return res.get_child("content");
        }
    private:
        int connect(){
            boost::system::error_code ec;
            mSocket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(mEurekaIp),mEurekaPort),ec);
            if( ec )
                return -1;
            boost::property_tree::ptree pt;
            pt.put("service name",mServiceName);
            pt = request("query service",pt);
            if(pt.count("service address") < 1){
                return -1;
            }
            std::string serviceAddr = pt.get<std::string>("service address");
            char ip[20];
            uint16_t port;
            sscanf(serviceAddr.c_str(),"%s:%hu",ip,&port);
            mSocket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip),port),ec);
            if( ec )
                return -1;
            
            return 0;
        }
    private:
        std::string mEurekaIp;
        uint16_t mEurekaPort;
        std::string mServiceName;
        boost::asio::io_context mIoContext;
        boost::asio::ip::tcp::socket mSocket;
        std::vector<char> mRecvBuffer;
        std::vector<char> mSendBuffer;

};
#endif//ECHO_CLIENT_H