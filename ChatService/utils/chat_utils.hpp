#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>

using sock     = boost::asio::ip::tcp::socket;
using sock_ptr = std::shared_ptr<sock>;

constexpr size_t BUFF_SIZE = 1024;
constexpr size_t MSG_PREFIX = sizeof(uint16_t);

boost::asio::ip::tcp::endpoint CreateAddress(const std::string& address);