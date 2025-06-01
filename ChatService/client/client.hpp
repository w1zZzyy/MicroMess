#pragma once

#include <chat_utils.hpp>

class Client
{
public:

    Client(const std::string& address);
    void launch();

private:

    boost::asio::io_context io;
    boost::asio::ip::tcp::endpoint ep;
    sock socket;


    void sender();
    void receiver();
    void handle_signals();
    void disconnect();
};