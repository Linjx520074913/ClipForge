cmake_minimum_required(VERSION 3.25)
project(cfcore LANGUAGES CXX CUDA)

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

# SDL2
set(SDL_INCLUDE_DIR ${DEPS_DIR}/sdl2/include)
set(SDL_LIB_DIR     ${DEPS_DIR}/sdl2/lib/x64)

# GLEW
set(GLEW_INCLUDE_DIR ${DEPS_DIR}/glew/include)
set(GLEW_LIB_DIR     ${DEPS_DIR}/glew/lib/Release/x64)

# Vulkan
set(VULKAN_INCLUDE_DIR ${DEPS_DIR}/vulkan/Include)
set(VULKAN_LIB_DIR     ${DEPS_DIR}/vulkan/Lib)

# WGPU
set(WGPU_INCLUDE_DIR ${DEPS_DIR}/wgpu/include/webgpu)
set(WGPU_LIB_DIR     ${DEPS_DIR}/wgpu/lib)

# CUDA
set(CUDA_INCLUDE_DIR "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/include")
set(CUDA_LIB_DIR     "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.2/lib/x64")

# ---------- include / link ----------
include_directories(
    ${DEPS_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${FFMPEG_INCLUDE_DIR}
    ${OPENCV_INCLUDE_DIR}
    ${SDL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${WGPU_INCLUDE_DIR}
    ${CUDA_INCLUDE_DIR}
)
link_directories(
    ${FFMPEG_LIB_DIR}
    ${OPENCV_LIB_DIR}
    ${SDL_LIB_DIR}
    ${GLEW_LIB_DIR}
    ${WGPU_LIB_DIR}
    ${CUDA_LIB_DIR}
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
    glew32
    SDL2
    cudart
    cuda
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
    SDL2
    wgpu_native
    cuda
    cudart
    d3d11
    dxgi
    d3dcompiler
)
