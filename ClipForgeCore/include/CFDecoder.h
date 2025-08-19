#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

struct CFFrame {
    int width = 0;
    int height = 0;
    int channels = 4;
    int length = 0;
    std::shared_ptr<uint8_t[]> data;
    double timestamp = 0.0;
};

struct CFFrameCacheEntry {
    double timestamp;
    std::shared_ptr<CFFrame> frame;
};

class CFDecoder {
public:
    CFDecoder();
    ~CFDecoder();

    bool open_video(const char* file_path);
    void close_video();
    std::shared_ptr<CFFrame> get_frame(double timestamp_ms);

private:
    void decode_loop();
    void push_to_display_queue(std::shared_ptr<CFFrame> frame);
    void cleanup();

    static double pts_to_ms(int64_t pts, AVRational tb) {
        return (pts == AV_NOPTS_VALUE) ? 0.0 : static_cast<double>(pts) * av_q2d(tb) * 1000.0;
    }

private:
    AVFormatContext* fmt_ctx_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    SwsContext* sws_ctx_ = nullptr;
    int best_video_stream_idx_ = -1;

    std::thread decode_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> eof_{false};

    mutable std::mutex display_mutex_;
    std::condition_variable decode_cv_;
    std::deque<CFFrameCacheEntry> display_queue_;
    std::shared_ptr<CFFrame> last_frame_;

    double low_watermark_ms_ = 200.0;
    double high_watermark_ms_ = 500.0;
    double hard_cap_ms_ = 800.0;
    double frame_interval_ms_ = 33.333;

    int width_ = 0;
    int height_ = 0;
    double fps_ = 30.0;
};
