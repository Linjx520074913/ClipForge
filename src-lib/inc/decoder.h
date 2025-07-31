#ifndef DECODER_DLL_H
#define DECODER_DLL_H

#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#define DLL_IMPORT extern "C" __declspec(dllimport)
#else
#define DLL_EXPORT extern "C"
#define DLL_IMPORT extern "C"
#endif

// ✅ 这个宏用于区分编译 DLL 还是调用 DLL
#ifdef DECODER_DLL_EXPORTS
#define DECODER_API DLL_EXPORT
#else
#define DECODER_API DLL_IMPORT
#endif

// ✅ 导出 hello_world 接口
DECODER_API const char* hello_world();

#endif // DECODER_DLL_H
