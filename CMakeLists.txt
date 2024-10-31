cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project (HTTP_Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(BOOST_ROOT "C:\\Program Files\\Boost\\boost_1_86_0\\boost_1_86_0")

find_package (Boost REQUIRED)

if (Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif ()

add_executable(http_server src/HTTP_Server.cpp)

target_link_libraries(http_server ${Boost_LIBRARIES})

target_compile_options(http_server PRIVATE "/utf-8")