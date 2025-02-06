set(SERVER_SOURCES
    src/HTTP_Server.cpp
    src/MessageHandler.cpp
    src/Session.cpp
    src/Compression.cpp
    src/ImageProcessor.cpp
    src/Params.cpp
)

add_executable(http_server ${SERVER_SOURCES})

find_package(OpenCV REQUIRED)

set(JPEG_DIR "${CMAKE_SOURCE_DIR}/Libs/libjpeg-turbo64")
set(JPEG_INCLUDE_DIR "${JPEG_DIR}/include")
set(JPEG_LIBRARY "${JPEG_DIR}/lib/jpeg.lib")

target_include_directories(http_server PRIVATE ${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR})
target_link_libraries(http_server Boost::program_options ${OpenCV_LIBS} ${JPEG_LIBRARY})