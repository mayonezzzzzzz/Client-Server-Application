cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project (HTTP_Server)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

set(JPEG_DIR "${CMAKE_SOURCE_DIR}/Libs/libjpeg-turbo64")
set(JPEG_INCLUDE_DIR "${JPEG_DIR}/include")
set(JPEG_LIBRARY "${JPEG_DIR}/lib/jpeg.lib")

include_directories(${JPEG_INCLUDE_DIR})

set(SERVER_SOURCES
    src/HTTP_Server.cpp
    src/MessageHandler.cpp
    src/Session.cpp
    src/Compression.cpp
)

add_executable(http_server ${SERVER_SOURCES})

target_link_libraries(http_server Boost::system ${JPEG_LIBRARY})