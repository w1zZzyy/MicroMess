#include "client.hpp"

#include <iostream>

using namespace boost::asio;

Client::Client(const std::string &address) : 
    socket(io), sig(io, SIGTERM, SIGINT),
    input(io, ::dup(STDIN_FILENO))
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
    async_read_until(
        input, buf, '\n',
        [this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec) {
                return;
            }

            std::istream is(&buf);
            std::string msg;
            std::getline(is, msg);
            send_message(msg);
        }
    );
}



void Client::send_message(const std::string &msg)
{
    U16 msg_len = std::min(msg.size(), BUFF_SIZE);
    U16 prefix = (U16(MessageFlag::TextAll) << 13) | msg_len;
    auto buf = std::make_shared<vec>(MSG_PREFIX + msg_len);

    memcpy(buf->data(), &prefix, MSG_PREFIX);
    memcpy(buf->data() + MSG_PREFIX, msg.data(), msg_len);

    async_write(
        socket,
        buffer(*buf),
        [this](const boost::system::error_code& ec, size_t bt)
        {
            if(ec) {
                return;
            }

            sender();
        }
    ); 
}



void Client::receiver()
{
    auto prefix = std::make_shared<vec>(MSG_PREFIX);

    async_read(
        socket,
        buffer(*prefix),
        [this, prefix]
        (const boost::system::error_code &ec, size_t bt)
        {
            if(ec) {
                return;
            }

            auto [len, flag] = DecodePrefix(*prefix);
            auto msg = std::make_shared<vec>(len);

            recv_message(msg);
        }
    );
}



void Client::recv_message(vec_ptr msg)
{
    async_read(
        socket,
        buffer(*msg),
        [this, msg]
        (const boost::system::error_code &ec, size_t bt)
        {
            if(ec) {
                return;
            }

            std::cout << std::string(msg->begin(), msg->end()) << "\n";

            receiver();
        }
    );
}



void Client::handle_signals()
{
    sig.async_wait(
        [this](const boost::system::error_code&, int)
        {
            constexpr auto flag = U16(MessageFlag::Logout) << 13;

            char bytes_flag[MSG_PREFIX];
            memcpy(bytes_flag, &flag, MSG_PREFIX);
            
            socket.send(
                buffer(bytes_flag)
            );

            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            socket.close();
            io.stop();
            input.close();
        }
    );
}
