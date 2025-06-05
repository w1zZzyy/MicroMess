#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <unordered_set>
#include <queue>

#include "chat_utils.hpp"
#include "message.hpp"


class Server
{
public:

    Server(const std::string& address);
    ~Server();

    void launch();

private:

    io_context io;
    ip::tcp::acceptor acc;
    ip::tcp::endpoint ep;

    std::vector<std::thread> workers;

    std::unordered_set<sock_ptr> clients;
    std::mutex clientsMTX, ioMTX;


    // adding new clients
    awaitable<void> start_accept();

    // receiving messages from client
    // and sending it to others
    awaitable<void> start_client(sock_ptr client);
};