cmake_minimum_required(VERSION 3.15)

project(FrameDLL)

set(CMAKE_CXX_STANDARD 20)

set(FFMPEG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/ffmpeg/include)
set(FFMPEG_LIB_DIR     ${PROJECT_SOURCE_DIR}/deps/ffmpeg/lib)

set(OPENCV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/opencv/release/include)
set(OPENCV_LIB_DIR     ${PROJECT_SOURCE_DIR}/deps/opencv/release/x64/vc15/lib)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${FFMPEG_INCLUDE_DIR}
    ${OPENCV_INCLUDE_DIR}
)
link_directories(
    ${FFMPEG_LIB_DIR}
    ${OPENCV_LIB_DIR}
)

add_library(${PROJECT_NAME} SHARED src/decoder.cpp)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    avcodec
    avformat
    avfilter
    avdevice
    avutil
    swresample
    swscale
    opencv_world455
)

set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(test src/test_decoder.cpp)
target_link_libraries(test PRIVATE ${PROJECT_NAME})