cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Client_Server_Application)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(Boost REQUIRED COMPONENTS program_options json CONFIG)
find_package(OpenCV REQUIRED)
find_package(libjpeg-turbo REQUIRED CONFIG)

message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")

add_subdirectory(server)
add_subdirectory(client)