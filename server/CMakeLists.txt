cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project (HTTP_Server)

if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif ()

add_executable(http_server src/HTTP_Server.cpp)

target_link_libraries(http_server ${Boost_LIBRARIES})