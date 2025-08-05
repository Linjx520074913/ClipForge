#pragma once

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

struct VideoFrame {
    int64_t pts_us;
    std::shared_ptr<cv::Mat> frame_data;
};

class VideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    bool Initialize(const char* file_path);
    void Uninitialize();

    std::shared_ptr<cv::Mat> GetFrame(int time_ms, int timeout_ms);

private:
    void DecodeThreadLoop();
    void DecodeWindow(int64_t start_pts_us);
    void Cleanup();

private:
    AVFormatContext* format_ctx_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    int video_stream_index_ = -1;
    SwsContext* sws_ctx_ = nullptr;
    AVPixelFormat current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    std::thread decode_thread_;
    std::mutex mutex_;
    std::condition_variable cv_decode_;
    std::condition_variable cv_frame_;

    std::deque<VideoFrame> frame_buffer_;
    const size_t max_cache_frames_ = 60;

    bool is_running_ = false;
    bool need_decode_ = false;

    int64_t target_pts_us_ = 0;

    int64_t current_window_start_us_ = 0;
    int64_t current_window_end_us_ = 0;

    static constexpr int64_t kDecodeWindowUs = 5 * 1000 * 1000; // 5秒窗口
    static constexpr int64_t kDropToleranceUs = 40 * 1000;      // 40ms 容忍误差
};
