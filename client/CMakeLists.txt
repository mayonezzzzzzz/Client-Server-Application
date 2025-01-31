cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(CLIENT_SOURCES
    src/HTTP_Client.cpp
    src/ImageHandler.cpp
    src/Logger.cpp
)

add_executable(http_client ${CLIENT_SOURCES})

target_link_libraries(http_client Boost::system)