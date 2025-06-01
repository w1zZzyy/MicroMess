#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <boost/asio.hpp>


using U16       =   uint16_t;
using sock      =   boost::asio::ip::tcp::socket;
using sock_ptr  =   std::shared_ptr<sock>;
using vec_ptr   =   std::shared_ptr<std::vector<char>>;


enum class MessageFlag : U16    // actually takes only 3 bits
{
    ERROR = 0,              // error on server side
    OK,                     // success server answere
    TextAll,                // text msg all users
    TextOne,                // text msg to one user
    TextGroup,              // text msg to the group of people
    Logout,                 // client disconnected
    Ping,                   // client sends 
    Pong,                   // server responds
};


constexpr size_t MSG_PREFIX = sizeof(U16); // 13 - len => 3 - flag
constexpr size_t BUFF_SIZE = 8191;


std::pair<size_t, MessageFlag> DecodePrefix(const std::vector<char>& prefix);
boost::asio::ip::tcp::endpoint CreateAddress(const std::string& address);

template<typename T>
void run(int argc, char* argv[]);

template <typename T>
inline void run(int argc, char *argv[])
{
    assert(argc == 2);
    
    try
    {   
        T sys(argv[1]);
        sys.launch();
    }

    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}
