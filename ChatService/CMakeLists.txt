cmake_minimum_required(VERSION 3.10)
project(Chat LANGUAGES CXX)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(UtilsLib STATIC
    utils/chat_utils.cpp utils/chat_utils.hpp
    utils/message.cpp utils/message.hpp
)
target_include_directories(UtilsLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/utils)
target_link_libraries(UtilsLib PUBLIC ${Boost_LIBRARIES})
target_compile_features(UtilsLib PUBLIC cxx_std_20)


add_library(ClientLib STATIC
    client/client.cpp
    client/client.hpp
)
target_include_directories(ClientLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/client
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)
target_link_libraries(ClientLib PUBLIC 
    ${Boost_LIBRARIES} 
    UtilsLib
)

add_library(ServerLib STATIC
    server/server.hpp server/server.cpp
)
target_include_directories(ServerLib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    utils
)
target_link_libraries(ServerLib PUBLIC 
    ${Boost_LIBRARIES}
    UtilsLib
)

add_executable(ClientExe client/main.cpp)
target_link_libraries(ClientExe PRIVATE ClientLib)

add_executable(ServerExe server/main.cpp)
target_link_libraries(ServerExe PRIVATE ServerLib)