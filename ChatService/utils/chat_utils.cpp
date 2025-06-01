#include "chat_utils.hpp"

#include <sstream>

using namespace boost::asio;

std::pair<size_t, MessageFlag> DecodePrefix(const std::vector<char> &prefix)
{
    U16 pref;
    memcpy(&pref, prefix.data(), MSG_PREFIX);

    size_t len = pref & 8191;
    MessageFlag flag = static_cast<MessageFlag>(pref >> 13);

    return {len, flag};
}

boost::asio::ip::tcp::endpoint CreateAddress(const std::string &address)
{
    std::stringstream ss(address);
    std::string ip, port;

    std::getline(ss, ip, ':');
    std::getline(ss, port);

    return ip::tcp::endpoint(ip::address::from_string(ip), std::stoi(port));
}