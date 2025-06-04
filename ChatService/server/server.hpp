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
    ~Server();

    void launch();

private:

    boost::asio::io_context io;
    boost::asio::ip::tcp::acceptor acc;
    boost::asio::ip::tcp::endpoint ep;

    std::vector<std::thread> workers;

    std::unordered_set<sock_ptr> clients;
    std::mutex clientsMTX, ioMTX;


    // adding new clients
    void start_accept();

    // receiving messages from client
    void start_client(sock_ptr client);

    // sends message to all clients
    void send_message(sock_ptr sender, vec_ptr msg, vec_ptr prefix);
};