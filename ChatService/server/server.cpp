#include "server.hpp"


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
    
    co_spawn(io, this->start_accept(), detached);

    size_t tc = std::thread::hardware_concurrency();
    workers.resize(tc);

    for(auto& worker : workers) {
        worker = std::thread([this](){io.run();});
    }
}

awaitable<void> Server::start_accept()
{
    for(;;)
    {
        auto new_client = std::make_shared<sock>(io);
        co_await acc.async_accept(*new_client, use_awaitable);

        {
            std::lock_guard<std::mutex> lg(clientsMTX);
            clients.insert(new_client);
        }

        co_spawn(new_client->get_executor(), this->start_client(new_client), detached);
    }

    co_return;
}

awaitable<void> Server::start_client(sock_ptr client)
{
    for(;;)
    {
        auto msg = Message::Create();

        co_await msg->read_prefix(*client);
        co_await msg->read_data(*client);


        switch (msg->getType())
        {
        case MessageType::Logout:
            {
            std::lock_guard<std::mutex> lg(ioMTX);
            std::cout << "---Client [" << msg->getName() << "] Disconnected---\n";
            }
            {
            std::lock_guard<std::mutex> lg(clientsMTX);
            clients.erase(client);  
            }

            client->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            client->close();
            continue;

        case MessageType::Login:
            {
            std::lock_guard<std::mutex> lg(ioMTX);
            std::cout << "---Client [" << msg->getName() << "] Connected---\n";
            }
            continue;

        default:
            break;
        }

        std::unique_lock<std::mutex> lg(clientsMTX);
        for(auto& receiver : clients)
        {
            if(receiver == client) continue;
            co_await msg->write(*receiver);
        }
    }

    co_return;
}
