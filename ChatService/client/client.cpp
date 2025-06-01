#include "client.hpp"

#include <iostream>

using namespace boost::asio;

Client::Client(const std::string &address) : socket(io)
{
    ep = CreateAddress(address);
    socket.connect(ep);
}

void Client::launch()
{
    handle_signals();
    sender();
    receiver();
    io.run();
}

void Client::sender()
{
    std::string msg;
    std::getline(std::cin, msg);
    
    U16 msg_len = std::min(msg.size(), BUFF_SIZE);
    U16 prefix = (U16(MessageFlag::TextAll) << 13) | msg_len;
    auto buf = std::make_shared<std::vector<char>>(MSG_PREFIX + msg_len);

    memcpy(buf->data(), &prefix, MSG_PREFIX);
    memcpy(buf->data() + MSG_PREFIX, msg.data(), msg_len);

    async_write(
        socket,
        buffer(*buf),
        [buf, msg_len, this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec || bt != msg_len) {
                disconnect();
            }

            sender();
        }
    );
}

void Client::receiver()
{
    auto prefix = std::make_shared<std::vector<char>>(MSG_PREFIX);
    async_read(
        socket,
        buffer(*prefix),
        [prefix, this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec || bt != MSG_PREFIX) {
                disconnect();
            }

            auto [len, flag] = DecodePrefix(*prefix);
            auto buf = std::make_shared<std::string>(len, '\0');

            async_read(
                socket,
                buffer(*buf),
                [buf, len, this](const boost::system::error_code& ec, size_t bt)
                {
                    if(ec || bt != len) {
                        disconnect();
                    }

                    std::cout << *buf << '\n';
                    
                    receiver();
                }
            );
        }
    );
}

void Client::handle_signals()
{
    signal_set sig(io, SIGINT, SIGTERM);
    
    sig.async_wait([&](boost::system::error_code, int) {
        disconnect();
    });
}

void Client::disconnect()
{
    auto flag = MessageFlag::Logout;

    char bytes_flag[MSG_PREFIX];
    memcpy(bytes_flag, (char*)(&flag), MSG_PREFIX);
    
    socket.send(
        buffer(bytes_flag)
    );

    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket.close();
    io.stop();
}
