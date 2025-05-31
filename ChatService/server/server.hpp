#pragma once

#include <thread>
#include <shared_mutex>
#include <vector>
#include <unordered_set>

#include "chat_utils.hpp"

class Server
{
public:

    Server(const std::string& address);

    void launch_threads(size_t counter = 0);

private:

    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acc;
    boost::asio::ip::tcp::endpoint ep;

    std::vector<std::thread> workers;
    std::unordered_set<sock_ptr> clients;
    std::shared_mutex clientsMTX;


    void start_accept();
    void start_recv(sock_ptr client);
    void read_body(size_t msg_len, sock_ptr client);
    void handle_disconnect(sock_ptr client);
    void handle_message(sock_ptr client, std::shared_ptr<std::vector<char>> buf);
};