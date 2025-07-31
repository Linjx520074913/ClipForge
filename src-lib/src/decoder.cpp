#define DECODER_DLL_EXPORTS  // 告诉编译器我们正在编译 DLL
#include "decoder.h"

const char* hello_world() {
    return "Hello World from FFmpeg DLL!";
}