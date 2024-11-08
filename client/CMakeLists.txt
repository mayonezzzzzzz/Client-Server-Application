cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project (HTTP_Client)

if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif ()

add_executable(http_client src/HTTP_Client.cpp)

target_link_libraries(http_client ${Boost_LIBRARIES})