#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H
#include <string.h>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>

#include <boost/asio.hpp>
class EchoClient{
    boost::asio::ip::udp::socket mSocket;
    boost::asio::ip::udp::endpoint mServerEndpoint;
    std::vector<char> mSendBuffer;
    std::vector<char> mRecvBuffer;

public:
    EchoClient(boost::asio::io_context& io_context ,const std::string& serverIp,uint16_t serverPort):mSocket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),mServerEndpoint(boost::asio::ip::address::from_string(serverIp) ,serverPort)
      ,mSendBuffer(65535),mRecvBuffer(65535){
        doReceive();
    }
    void doReceive()
     {
       mSocket.async_receive_from(
           boost::asio::buffer(mRecvBuffer.data(), mRecvBuffer.size()-1), mServerEndpoint,
           [this](boost::system::error_code ec, std::size_t bytes_recvd)
           {
             if (!ec && bytes_recvd > 0)
             {

             }
             doReceive();
           });
     }
    void sendResqust(const boost::property_tree::ptree& pt){
        std::stringstream ss;
        boost::property_tree::write_json(ss, pt);
        strcpy(mSendBuffer.data(),ss.str().c_str());
        mSocket.async_send_to(
                   boost::asio::buffer(mSendBuffer.data(), strlen(mSendBuffer.data())-1), mServerEndpoint,
                   [this](boost::system::error_code ec, std::size_t bytes_sent)
                   {
                        std::cout<<ec.message()<<":"<<bytes_sent<<std::endl;
                     //do_receive();

                   });
    }

};
#endif // ECHOCLIENT_H
