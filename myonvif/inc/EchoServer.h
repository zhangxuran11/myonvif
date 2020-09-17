#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H
#include <boost/asio.hpp>
class EchoServer{
    boost::asio::ip::udp::endpointudp::socket socket_;
};
#endif//ECHO_SERVER_H