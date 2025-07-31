#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

int main() {
    // 使用 av_version_info 获取版本字符串
    std::cout << "FFmpeg version: " << av_version_info() << std::endl;

    // 还可以打印具体库的版本号
    unsigned libavcodec_ver = avcodec_version();
    std::cout << "libavcodec version: "
              << AV_VERSION_MAJOR(libavcodec_ver) << "."
              << AV_VERSION_MINOR(libavcodec_ver) << "."
              << AV_VERSION_MICRO(libavcodec_ver) << std::endl;

    return 0;
}