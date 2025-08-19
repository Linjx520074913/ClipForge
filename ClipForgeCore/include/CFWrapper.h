#ifndef __CF_WRAPPER_H__
#define __CF_WRAPPER_H__

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#define DLL_IMPORT extern "C" __declspec(dllimport)
#else
#define DLL_EXPORT extern "C"
#define DLL_IMPORT extern "C"
#endif

#include <cstdint>

struct CFFrame;

// 这个宏用于区分编译 DLL 还是调用 DLL
#ifdef CF_CORE_DLL
    #define CF_CORE_API DLL_EXPORT
#else
    #define CF_CORE_API DLL_IMPORT
#endif

CF_CORE_API void     cf_player_open(const char* file_path);
CF_CORE_API void     cf_player_close();
CF_CORE_API CFFrame* cf_player_get_frame_at(int64_t timestamp);


#endif
