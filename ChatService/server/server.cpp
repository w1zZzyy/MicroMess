#include "server.hpp"

using namespace boost::asio;



Server::Server(const std::string &address) : io(), acc(io)
{
    ep = CreateAddress(address);

    acc.open(ep.protocol());
    acc.set_option(boost::asio::socket_base::reuse_address(true));
    acc.bind(ep);
}



Server::~Server()
{
    for(auto& t : workers) {
        if(t.joinable()) { t.join(); }
    }
}



void Server::launch()
{
    acc.listen();
    start_accept();

    size_t tc = std::thread::hardware_concurrency();
    workers.resize(tc);

    for(auto& worker : workers) {
        worker = std::thread([this](){io.run();});
    }
}



void Server::start_accept()
{
    auto new_client = std::make_shared<sock>(io);
    acc.async_accept(
        *new_client,
        [this, new_client](const boost::system::error_code &ec)
        {
            if(ec) {
                return;
            }

            {
                std::lock_guard<std::mutex> lg(clientsMTX);
                clients.insert(new_client);
            }

            {
                std::lock_guard<std::mutex> lg(ioMTX);
                std::cout << "------Client Connected------\n";
            }

            start_client(new_client);
            start_accept();
        }
    ); 
}



void Server::start_client(sock_ptr client)
{
    auto prefix = std::make_shared<vec>(MSG_PREFIX);

    async_read(
        *client,
        buffer(*prefix),
        [this, client, prefix]
        (const boost::system::error_code &ec, size_t bt)
        {
            if(ec) {
                return;
            }


            auto [len, flag] = DecodePrefix(*prefix);

            if(flag == MessageFlag::Logout) {
                {
                    std::lock_guard<std::mutex> lg(ioMTX);
                    std::cout << "------Client Disconnected------\n";
                }

                {
                    std::lock_guard<std::mutex> lg(clientsMTX);
                    clients.erase(client);
                }

                return;
            }

            auto msg = std::make_shared<vec>(len);

            async_read(
                *client,
                buffer(*msg),
                [this, client, msg, prefix]
                (const boost::system::error_code &ec, size_t bt)
                {
                    if(ec) {
                        return;
                    }

                    send_message(client, msg, prefix);
                    start_client(client);
                }
            );
        }
    );
}

void Server::send_message(sock_ptr sender, vec_ptr msg, vec_ptr prefix)
{
    std::lock_guard<std::mutex> lg(clientsMTX);
    for(auto& client : clients)
    {
        if(client == sender) continue;

        async_write(
            *client,
            buffer(*prefix),
            [client, msg](const boost::system::error_code &ec, size_t bt)
            {
                if(ec) {
                    return;
                }

                async_write(
                    *client,
                    buffer(*msg),
                    [](const boost::system::error_code& ec, size_t){
                        if(ec) {
                            return;
                        }
                    }
                );
            }
        );
    }
}
