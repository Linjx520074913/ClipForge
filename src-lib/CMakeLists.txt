cmake_minimum_required(VERSION 3.10)

set(Target "FFmpeg")

# 项目名称
project(FFmpeg)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加可执行文件
add_executable(${Target} src/main.cpp)


# 添加头文件搜索路径
target_include_directories(${Target} PRIVATE "deps/ffmpeg/include")

# 添加库文件搜索路径
target_link_directories(${Target} PRIVATE "deps/ffmpeg/lib")

# 链接FFmpeg相关库
target_link_libraries(${Target} PRIVATE avformat avcodec avutil swscale)