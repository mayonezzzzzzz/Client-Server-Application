set(SERVER_SOURCES
    src/HTTP_Server.cpp
    src/MessageHandler.cpp
    src/Session.cpp
    src/Compression.cpp
    src/ImageProcessor.cpp
    src/Params.cpp
)

add_executable(http_server ${SERVER_SOURCES})

target_include_directories(http_server PRIVATE ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(http_server Boost::program_options ${OpenCV_LIBS} libjpeg-turbo::jpeg)