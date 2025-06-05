#pragma once

#include <string_view>
#include <chat_utils.hpp>
#include <boost/asio/awaitable.hpp>

enum class MessageType : U8
{
    None,
    Message,
    Login,
    Logout,
    Ping, Pong
};


struct MessageHeader
{
    U16 msg_len;
    MessageType flag;
    U8 name_len;
};


constexpr size_t mtMem = sizeof(MessageType);
constexpr size_t mlMem = sizeof(U16);
constexpr size_t nlMem = sizeof(U8);


class Message;
using msg_ptr = std::shared_ptr<Message>;



class Message
{
public:

    static msg_ptr Create();
    static msg_ptr Create(
        std::string_view    name, 
        std::string_view    message, 
        MessageType         flag
    );

    awaitable<void> read_prefix(sock& client);
    awaitable<void> read_data(sock& client);

    awaitable<void> write(sock& client);
    
    MessageType         getType() const noexcept {return header.flag;};
    std::string_view    getName() const;
    std::string_view    getMessage() const;

private:

    MessageHeader   header;
    char            prefix[sizeof(MessageHeader)];
    vec             data;
};