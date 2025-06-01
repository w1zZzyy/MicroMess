#include "server.hpp"

#include <string_view>

using namespace boost::asio;

Server::Server(const std::string &address) : io(), acc(io)
{
    ep = CreateAddress(address);

    acc.open(ep.protocol());
    acc.set_option(boost::asio::socket_base::reuse_address(true));
    acc.bind(ep);
}



void Server::launch()
{
    acc.listen();
    start_accept();
    handle_signals();

    size_t tc = std::thread::hardware_concurrency();
    workers.resize(tc);

    for(auto& worker : workers) {
        worker = std::thread([this](){io.run();});
    }
}

void Server::start_accept()
{
    auto client = std::make_shared<sock>(io);
    acc.async_accept(*client, 
        [this, client](const boost::system::error_code& ec)
        {
            if(!ec) 
            {      
                recv_message(client);

                std::lock_guard<std::mutex> ul(clientsMTX);
                clients.insert(client);
            }

            start_accept();
        }
    );
}

void Server::recv_message(sock_ptr sender)
{
    /* 
    message format: 
        -3  bits: msg flag
        -13 bits: msg len
        -msg len bytes: msg

        fl     |     len         |          msg
        3 bits |     13 bits     |          len bytes
    */


    auto buf = std::make_shared<std::vector<char>>(MSG_PREFIX);

    // reading prefix first 16 bits
    async_read(
        *sender,
        buffer(*buf),
        [this, sender, buf](const boost::system::error_code& ec, size_t bt)
        {
            if(ec || bt != MSG_PREFIX)
            {
                handle_disconnect(sender);
                return;
            }

            auto [len, flag] = DecodePrefix(*buf);

            if(flag == MessageFlag::Logout) {
                handle_disconnect(sender);
                return;
            }


            buf->resize(MSG_PREFIX + len);

            // reading message
            async_read(
                *sender,
                buffer(buf->data() + MSG_PREFIX, len),
                [this, len, sender, buf](const boost::system::error_code& ec, size_t bt)
                {
                    if(ec || bt != len)
                    {
                        handle_disconnect(sender);
                        return;
                    }

                    // sending message to all the clients
                    send_message(sender, buf);

                    // and again waiting for client to send msg
                    recv_message(sender);
                }
            );
        }
    );
}

void Server::send_message(sock_ptr sender, vec_ptr buf)
{
    std::lock_guard<std::mutex> sl(clientsMTX);
    for(auto client : clients)
    {
        if(client == sender) continue;  

        async_write(
            *client,
            buffer(*buf),
            [this, client](const boost::system::error_code& ec, size_t)
            {
                if(ec)
                {
                    handle_disconnect(client);
                    return;
                }
            }
        );
    }
}

void Server::handle_disconnect(sock_ptr client)
{
    std::lock_guard<std::mutex> ul(clientsMTX);
    clients.erase(client);
}

void Server::handle_signals()
{
    auto sig = std::make_shared<signal_set>(io, SIGINT, SIGTERM);
    
    sig->async_wait([this](boost::system::error_code, int) 
    {
        std::cout << "SERVER STOPPED!\n";

        {
            std::lock_guard<std::mutex> ul(clientsMTX);
            clients.clear();
        }
        
        acc.close();
        io.stop();
        
        for(auto& t : workers) {
            if(t.joinable()) { t.join(); }
        }
    });
}
