cmake_minimum_required(VERSION 3.15)

project(ffmpeg_demo)

set(CMAKE_CXX_STANDARD 17)

set(FFMPEG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/ffmpeg/include)
set(FFMPEG_LIB_DIR     ${PROJECT_SOURCE_DIR}/deps/ffmpeg/lib)

include_directories(${FFMPEG_INCLUDE_DIR})
link_directories(${FFMPEG_LIB_DIR})

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    avcodec
    avformat
    avfilter
    avdevice
    avutil
    swresample
    swscale
)