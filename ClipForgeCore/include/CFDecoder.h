#pragma once

#include "CFTypes.h"
#include "CFFrameCache.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}


class CFDecoder
{
public:

    void open_video(const char* file_path);
    void close_video();

    void cleanup();

    std::shared_ptr<CFFrame> get_frame(double timestampe);

    void decode_loop();

private:
    std::thread decode_thread_;
    std::mutex mutex_;

    CFFrameCache frame_cache_{100};

    AVFormatContext* fmt_ctx_   = nullptr;
    AVCodecContext*  codec_ctx_ = nullptr;
    SwsContext*      sws_ctx_   = nullptr;

    int best_video_stream_idx_ = -1;
};