#pragma once

#include <chat_utils.hpp>

class Client
{
public:

    Client(const std::string& address);
    void launch();

private:

    boost::asio::io_context                 io;
    boost::asio::posix::stream_descriptor   input;
    boost::asio::ip::tcp::endpoint          ep;
    sock                                    socket;
    boost::asio::signal_set                 sig;
    boost::asio::streambuf                  buf;

    void sender();
    void send_message(const std::string& msg);
    void receiver();
    void recv_message(vec_ptr msg);
    void handle_signals();
};