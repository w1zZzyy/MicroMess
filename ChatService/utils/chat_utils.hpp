#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <boost/asio.hpp>


using namespace boost::asio;


ip::tcp::endpoint CreateAddress(const std::string& address);



using U8        =   uint8_t;
using U16       =   uint16_t;
using U32       =   uint32_t;
using sock      =   ip::tcp::socket;
using sock_ptr  =   std::shared_ptr<sock>;
using vec       =   std::vector<char>;
using vec_ptr   =   std::shared_ptr<vec>;
