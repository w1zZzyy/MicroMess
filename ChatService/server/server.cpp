#include "server.hpp"

#include <array>
#include <functional>

using namespace boost::asio;

Server::Server(const std::string &address) : io(), acc(io)
{
    ep = CreateAddress(address);

    acc.open(ep.protocol());
    acc.set_option(boost::asio::socket_base::reuse_address(true));
    acc.bind(ep);
    acc.listen();

    start_accept();
}

void Server::launch_threads(size_t counter)
{
    size_t tc = (counter != 0) 
            ? counter 
            : std::thread::hardware_concurrency();
    workers.resize(tc);

    for(auto& worker : workers) {
        worker = std::thread([this](){io.run();});
    }
}

void Server::start_accept()
{
    auto client = std::make_shared<sock>(io);
    acc.async_accept(*client, 
        [this, client](const boost::system::error_code& ce)
        {
            if(!ce) 
            {      
                start_recv(client);

                std::unique_lock<std::shared_mutex> ul(clientsMTX);
                clients.insert(client);
            }

            start_accept();
        }
    );
}

void Server::start_recv(sock_ptr client)
{
    auto prefix = std::make_shared<std::array<char, MSG_PREFIX>>();

    async_read(*client, buffer(*prefix), 
        [prefix, client, this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec) {
                handle_disconnect(client);
                return;
            }

            if(bt == MSG_PREFIX)
            {
                uint16_t msg_len;
                std::memcpy(&msg_len, prefix->data(), MSG_PREFIX);

                read_body(msg_len, client);
            }
        }
    );     
}

void Server::read_body(size_t msg_len, sock_ptr client)
{
    if(msg_len == 0) start_recv(client);

    size_t mem = std::min(msg_len, BUFF_SIZE);
    auto msg = std::make_shared<std::vector<char>>(mem);

    async_read(*client, buffer(*msg), 
        [client, msg, msg_len, this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec) {
                handle_disconnect(client);
                return;
            }

            else if(bt <= msg_len) {
                handle_message(client, msg);
                read_body(msg_len - bt, client);
            }

            else { // bt > msg_len
                handle_message(client, msg);
                read_body(0, client);
            }
        }
    );
}

void Server::handle_message(sock_ptr sender, std::shared_ptr<std::vector<char>> buf)
{
    std::shared_lock<std::shared_mutex> sl(clientsMTX);

    for(auto client : clients)
    {
        if(client == sender) continue;

        client->async_send(
            buffer(*buf),
            [this, client, buf](const boost::system::error_code& ec, size_t bt)
            {
                if(ec) {
                    handle_disconnect(client);
                    return;
                }
            }
        );
    }
}

void Server::handle_disconnect(sock_ptr client)
{
    {
        std::unique_lock<std::shared_mutex> ul(clientsMTX);
        clients.erase(client);
    }
    // client send
}
