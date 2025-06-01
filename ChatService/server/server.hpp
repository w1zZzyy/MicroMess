#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <queue>

#include "chat_utils.hpp"


class Server
{
public:

    Server(const std::string& address);
    void launch();

private:

    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor acc;
    boost::asio::ip::tcp::endpoint ep;

    std::vector<std::thread> workers;

    std::unordered_set<sock_ptr> clients;
    std::mutex clientsMTX;


    // welcoming new clients
    void start_accept();

    // server receives message
    // that was sended by client(sender)
    // after that calls send_message()
    void recv_message(sock_ptr sender);

    // server sends message to clients 
    void send_message(sock_ptr sender, vec_ptr msg);

    // disconnecting client
    void handle_disconnect(sock_ptr client);

    // for safety programm end
    // (Ctrl+C in command line)
    void handle_signals();
};