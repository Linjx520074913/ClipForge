cmake_minimum_required(VERSION 3.25)
project(cfcore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES native)

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

# ---------- 主库 ----------
add_library(${PROJECT_NAME} SHARED
    src/CFPlayer.cpp
    src/CFWrapper.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    avcodec
    avformat
    avfilter
    avdevice
    avutil
    swresample
    swscale
    opencv_world455
    Dbghelp
    d3d11
    dxgi
    d3dcompiler
)

set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ---------- 示例可执行 ----------
add_executable(sample
    examples/sample.cpp
)

# NVCC 编译选项
set_target_properties(sample PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_ARCHITECTURES "native"
)

target_compile_options(sample PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--use_fast_math>
)

target_link_libraries(sample PRIVATE
    ${PROJECT_NAME}
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
)
