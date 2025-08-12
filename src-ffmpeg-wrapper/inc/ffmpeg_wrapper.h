#ifndef FFMPEG_WRAPPER_H_
#define FFMPEG_WRAPPER_H_

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#define DLL_IMPORT extern "C" __declspec(dllimport)
#else
#define DLL_EXPORT extern "C"
#define DLL_IMPORT extern "C"
#endif

// 这个宏用于区分编译 DLL 还是调用 DLL
#ifdef FFMPEG_WRAPPER_DLL
    #define WRAPPER_API DLL_EXPORT
#else
    #define WRAPPER_API DLL_IMPORT
#endif

#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}



WRAPPER_API const char* get_version();

WRAPPER_API void get_media_meta(const char* filePath);

WRAPPER_API void open_video(const char* filePath);

WRAPPER_API void get_frame(int millisecond);

WRAPPER_API void close_video();

WRAPPER_API const char* get_av_meta_data(const char* file_path);
WRAPPER_API void        free_av_meta_data(const char* ptr);

#endif
