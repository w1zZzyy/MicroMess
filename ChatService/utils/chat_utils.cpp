#include "chat_utils.hpp"

#include <sstream>

using namespace boost::asio;

boost::asio::ip::tcp::endpoint CreateAddress(const std::string &address)
{
    std::stringstream ss(address);
    std::string ip, port;

    std::getline(ss, ip, ':');
    std::getline(ss, port);

    return ip::tcp::endpoint(ip::address::from_string(ip), std::stoi(port));
}