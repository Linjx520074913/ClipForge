cmake_minimum_required(VERSION 3.25)
set(CMAKE_TOOLCHAIN_FILE "D:/code/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")

project(cfcore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(websocketpp CONFIG REQUIRED)

# ---------- 外部依赖路径 ----------
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/deps)

# FFmpeg
set(FFMPEG_INCLUDE_DIR ${DEPS_DIR}/ffmpeg/include)
set(FFMPEG_LIB_DIR     ${DEPS_DIR}/ffmpeg/lib)

# OpenCV
set(OPENCV_INCLUDE_DIR ${DEPS_DIR}/opencv/release/include)
set(OPENCV_LIB_DIR     ${DEPS_DIR}/opencv/release/x64/vc15/lib)

# ---------- include / link ----------
include_directories(
    ${DEPS_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIR}
    ${OPENCV_INCLUDE_DIR}
)
link_directories(
    ${FFMPEG_LIB_DIR}
    ${OPENCV_LIB_DIR}
)

# ---------- 示例可执行 ----------
add_executable(sample
    examples/sample.cpp
    examples/application.cpp
    examples/dx11renderer.cpp
    examples/decoder.cpp
)

target_link_libraries(sample PRIVATE
    avcodec
    avformat
    avfilter
    avdevice
    avutil
    swresample
    swscale
    opencv_world455
    d3d11
    dxgi
    d3dcompiler
    websocketpp::websocketpp
)
