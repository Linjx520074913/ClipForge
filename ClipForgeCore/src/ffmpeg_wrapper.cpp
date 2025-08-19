#define FFMPEG_WRAPPER_DLL

// #include "clipforge_decoder.h"
#include "CFDecoder.h"
#include "ffmpeg_wrapper.h"

#include <opencv2/opencv.hpp>


using namespace std;

// const char* get_version()
// {
//     return CFDecoder::get_version();
// }

// const char* get_meta_data(const char* file_path)
// {
//     return CFDecoder::get_av_meta_data(file_path);
// }

// void free_meta_data(const char* ptr)
// {
//     return CFDecoder::free_av_meta_data(ptr);
// }

CFDecoder g_cf_decoder;

void open_video(const char* file_path)
{
    g_cf_decoder.open_video(file_path);
}

// void get_frame(int millisecond)
// {

// }

CFFrame* get_current_frame(double millisecond)
{
    return g_cf_decoder.get_frame(millisecond).get();
}

void free_frame(CFFrame* frame)
{
    // g_cf_decoder.free_frame(frame);
}

void close_video()
{

}