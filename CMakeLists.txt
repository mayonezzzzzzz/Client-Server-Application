cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Client_Server_Application)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/Libs/boost_1_86_0")
find_package(Boost REQUIRED COMPONENTS system)

add_subdirectory(server)
add_subdirectory(client)