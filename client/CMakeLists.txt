set(CLIENT_SOURCES
    src/HTTP_Client.cpp
    src/ImageHandler.cpp
    src/Logger.cpp
    src/Params.cpp
)

add_executable(http_client ${CLIENT_SOURCES})

target_include_directories(http_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(http_client Boost::program_options Boost::json)