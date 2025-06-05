#pragma once

#include <chat_utils.hpp>

class Client
{
public:

    Client(const std::string& address, const std::string& name);
    void launch();

private:

    io_context                 io;
    posix::stream_descriptor   input;
    ip::tcp::endpoint          ep;
    sock                       socket;
    signal_set                 sig;
    streambuf                  buf;
    std::string                name;


    awaitable<void> sender();
    awaitable<void> receiver();
    awaitable<void> handle_signals();
};