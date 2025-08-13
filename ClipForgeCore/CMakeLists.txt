cmake_minimum_required(VERSION 3.15)

project(cfcore)

set(CMAKE_CXX_STANDARD 20)

set(FFMPEG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/ffmpeg/include)
set(FFMPEG_LIB_DIR     ${PROJECT_SOURCE_DIR}/deps/ffmpeg/lib)

set(OPENCV_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/opencv/release/include)
set(OPENCV_LIB_DIR     ${PROJECT_SOURCE_DIR}/deps/opencv/release/x64/vc15/lib)

set(DEPS_INCLUDE_DIR   ${PROJECT_SOURCE_DIR}/deps)

include_directories(
    ${DEPS_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIR}
    ${OPENCV_INCLUDE_DIR}
)
link_directories(
    ${FFMPEG_LIB_DIR}
    ${OPENCV_LIB_DIR}
)

add_library(${PROJECT_NAME} SHARED src/CFDecoder.cpp src/CFFrameCache.cpp)

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

add_executable(sample examples/sample.cpp src/CFDecoder.cpp src/CFFrameCache.cpp)
target_link_libraries(
    sample PRIVATE
    avcodec
    avformat
    avfilter
    avdevice
    avutil
    swresample
    swscale
    opencv_world455
)