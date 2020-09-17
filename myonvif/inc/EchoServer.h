#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>

#include <boost/asio.hpp>

class EchoServer{
    boost::asio::ip::udp::socket mSocket;
    boost::asio::ip::udp::endpoint mSenderEndpoint;
    std::vector<char> mRecvBuffer;
    typedef void (*Processer)(const boost::property_tree::ptree&);
    std::map<std::string,Processer> mProcesserSet;
    void doReceive()
     {
       mSocket.async_receive_from(
           boost::asio::buffer(mRecvBuffer.data(), mRecvBuffer.size()-1), mSenderEndpoint,
           [this](boost::system::error_code ec, std::size_t bytes_recvd)
           {
             if (!ec && bytes_recvd > 0)
             {
                 boost::property_tree::ptree pt;
                 std::stringstream sstream(mRecvBuffer.data());
                 boost::property_tree::json_parser::read_json(sstream, pt);
                 std::string cmd = pt.get<std::string>("cmd");
                 if(mProcesserSet.count(cmd) > 0){
                     mProcesserSet[cmd](pt);
                 }
             }
             doReceive();
           });
     }

//     void do_send(std::size_t length)
//     {
//       socket_.async_send_to(
//           boost::asio::buffer(data_, length), sender_endpoint_,
//           [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
//           {
//             do_receive();

//           });
//     }
public:
    EchoServer(boost::asio::io_context& io_context ,uint16_t port):mSocket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port)),mRecvBuffer(0x10000){
        doReceive();
    }
    void registerProcesser(const std::string& cmd,Processer pfn){
        mProcesserSet[cmd] = pfn;

    }
};
#endif//ECHO_SERVER_H
