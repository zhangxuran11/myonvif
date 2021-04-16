#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>

#include <boost/asio.hpp>
#include <boost/asio/impl/use_future.hpp>
#include <boost/asio/system_executor.hpp>
#include <boost/exception/all.hpp>

#include <string>
#include <iostream>
#include <memory>
#include <functional>

#include "EchoClient.h"

class EchoServer{
    class Session
    : public std::enable_shared_from_this<Session>{
        public:
            Session(EchoServer &server,boost::asio::ip::tcp::socket& socket):mServer(server),mSocket(std::move(socket)){
                //std::cout<<"Session()"<<std::endl;
            }
            ~Session(){
                //std::cout<<"~Session()"<<std::endl;
            }
            void doRead(){
                auto self = shared_from_this();
                mRecvBuffer.resize(4);
                boost::asio::async_read(mSocket,boost::asio::buffer(mRecvBuffer, mRecvBuffer.size()),
                [this, self](boost::system::error_code ec, std::size_t length)
                {
                    if (!ec && length > 0)
                    {
                        std::uint32_t len = * reinterpret_cast<std::uint32_t*>(mRecvBuffer.data());
                        mRecvBuffer.resize(len);
                        auto self = shared_from_this();
                        boost::asio::async_read(mSocket,boost::asio::buffer(mRecvBuffer, mRecvBuffer.size()),
                        [this, self](boost::system::error_code ec, std::size_t length)
                        {
                            if (!ec && length > 0){
                                boost::property_tree::ptree pt;
                                std::stringstream sstream(std::string(mRecvBuffer.data(),mRecvBuffer.size()));
                                boost::property_tree::json_parser::read_json(sstream, pt);
                                std::string cmd;
                                try{
                                    cmd = pt.get<std::string>("topic");
                                }
                                catch (boost::exception& e){
                                    std::cout<<boost::diagnostic_information(e)<<std::endl;
                                }
                                boost::property_tree::ptree res;
                                if(mServer.mProcesserSet.count(cmd) > 0){
                                    res.put_child("content",std::move(mServer.mProcesserSet[cmd](pt.get_child("content"))));
                                }
                                else{
                                    res.put_child("content",std::move(boost::property_tree::ptree()));
                                }
                                
                                sstream.str("");
                                std::stringstream ss;
                                boost::property_tree::json_parser::write_json(sstream,res);
                                mSendBuffer.resize(sstream.str().size()+4);
                                *reinterpret_cast<std::uint32_t*>(mSendBuffer.data()) = sstream.str().size();
                                memcpy(mSendBuffer.data()+4,sstream.str().data(),sstream.str().size());
                                boost::asio::async_write(mSocket,boost::asio::buffer(mSendBuffer.data(),mSendBuffer.size()),
                                    [this,self](boost::system::error_code ec, std::size_t length){
                                        if(!ec){
                                            doRead();
                                        }
                                        else{ //应答失败
                                        }
                                });
                            }
                        });
                    }
                });
            }
        private:
            EchoServer& mServer;
            boost::asio::ip::tcp::socket mSocket;
            std::vector<char> mRecvBuffer;
            std::vector<char> mSendBuffer;
    };
    void doAccept(){
        mAcceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
          if (!ec)
          {
              std::make_shared<Session>(*this,socket)->doRead();
          }
          doAccept();
        });

    }

public:
    EchoServer(uint16_t port) :mAcceptor(mIoContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        std::cout<<"address:"<<mAcceptor.local_endpoint().address().to_string()<<std::endl;
    }
    uint16_t port()const {
        return mAcceptor.local_endpoint().port();
    }
    void registerProcesser(const std::string& cmd,std::function<boost::property_tree::ptree(const boost::property_tree::ptree&)> processer){
        mProcesserSet[cmd] = processer;
    }
    void run(){
        doAccept();
        mIoContext.run();
    }
private:
    boost::asio::io_context mIoContext;
    boost::asio::ip::tcp::acceptor mAcceptor;
    std::vector<char> mRecvBuffer;
    std::vector<char> mSendBuffer;
    std::map<std::string,std::function<boost::property_tree::ptree (const boost::property_tree::ptree&)> > mProcesserSet;
};
#endif  //ECHO_SERVER_H
