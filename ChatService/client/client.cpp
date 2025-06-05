#include "client.hpp"
#include <message.hpp>
#include <iostream>

using namespace boost::asio;

Client::Client(const std::string &address, const std::string& name) : 
    name(name),
    socket(io), sig(io, SIGTERM, SIGINT),
    input(io, ::dup(STDIN_FILENO))
{ ep = CreateAddress(address); }



void Client::launch()
{
    socket.connect(ep);

    auto msg_con = Message::Create(name, "", MessageType::Login);
    std::cout << msg_con->getName() << "\n";
    co_spawn(io, msg_con->write(socket), detached);

    co_spawn(io, this->handle_signals(), detached);
    co_spawn(io, this->sender(), detached);
    co_spawn(io, this->receiver(), detached);

    io.run();
}


awaitable<void> Client::sender()
{
    for(;;)
    {
        co_await async_read_until(input, buf, '\n', use_awaitable);

        std::istream is(&buf);
        std::string message;
        std::getline(is, message);

        auto msg = Message::Create(name, message, MessageType::Message);
        co_await msg->write(socket);
    }

    co_return;
}

awaitable<void> Client::receiver()
{
    for(;;)
    {
        auto msg = Message::Create();

        co_await msg->read_prefix(socket);
        co_await msg->read_data(socket);

        std::cout << msg->getName() << ": " << msg->getMessage() << '\n';
    }

    co_return;
}

awaitable<void> Client::handle_signals()
{
    co_await sig.async_wait(use_awaitable);

    auto msg = Message::Create(name, "", MessageType::Logout);
    co_await msg->write(socket);

    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket.close();
    io.stop();
    input.close();

    co_return;
}
