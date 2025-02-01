cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(JPEG_DIR "${CMAKE_SOURCE_DIR}/Libs/libjpeg-turbo64")
set(JPEG_INCLUDE_DIR "${JPEG_DIR}/include")
set(JPEG_LIBRARY "${JPEG_DIR}/lib/jpeg.lib")

include_directories(${JPEG_INCLUDE_DIR})

set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/Libs/opencv/build")
set(OpenCV_INCLUDE_DIR "${OpenCV_DIR}/include")
set(OpenCV_LIBRARY "${OpenCV_DIR}/x64/vc16/lib/opencv_world4100.lib")

include_directories(${OpenCV_INCLUDE_DIR})

set(SERVER_SOURCES
    src/HTTP_Server.cpp
    src/MessageHandler.cpp
    src/Session.cpp
    src/Compression.cpp
    src/ImageProcessor.cpp
    src/Params.cpp
)

add_executable(http_server ${SERVER_SOURCES})

target_link_libraries(http_server Boost::system ${JPEG_LIBRARY} ${OpenCV_LIBRARY})