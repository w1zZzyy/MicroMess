#include "message.hpp"

#include <coroutine>

msg_ptr Message::Create()
{
    return std::make_shared<Message>();
}

msg_ptr Message::Create(
    std::string_view    name, 
    std::string_view    message, 
    MessageType         flag
)
{
    auto msg = Create();

    auto& h = msg->header;
    auto& p = msg->prefix;
    auto& d = msg->data;

    h.flag        =   flag;
    h.msg_len     =   message.size();
    h.name_len    =   name.size();

    memcpy(p, &h.flag, mtMem);
    memcpy(p + mtMem, &h.name_len, nlMem);
    memcpy(p + mtMem + nlMem, &h.msg_len, mlMem);

    d.resize(h.name_len + h.msg_len);

    memcpy(d.data(), name.data(), h.name_len);
    memcpy(d.data() + h.name_len, message.data(), h.msg_len);
    
    return msg;
}

awaitable<void> Message::read_prefix(sock& client)
{
    co_await async_read(client, buffer(prefix), use_awaitable);

    memcpy(&header.flag, prefix, mtMem);
    memcpy(&header.name_len, prefix + mtMem, nlMem);
    memcpy(&header.msg_len, prefix  + mtMem + nlMem, mlMem);

    co_return;
}

awaitable<void> Message::read_data(sock &client)
{
    data.resize(header.name_len + header.msg_len);
    co_await async_read(client, buffer(data), use_awaitable);
    co_return;
}



awaitable<void> Message::write(sock& client)
{
    co_await async_write(client, buffer(prefix), use_awaitable);
    co_await async_write(client, buffer(data), use_awaitable);
    co_return;
}



std::string_view Message::getName() const
{
    return std::string_view(
        data.begin(),
        data.begin() + header.name_len
    );
}

std::string_view Message::getMessage() const
{
    return std::string_view(
        data.begin() + header.name_len,
        data.end()
    );
}
