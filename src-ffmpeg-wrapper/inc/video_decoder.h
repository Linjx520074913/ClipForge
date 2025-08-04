#pragma once
#include <opencv2/opencv.hpp>
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

struct VideoFrame {
    int64_t pts_us;
    std::shared_ptr<cv::Mat> frame_data;
};

class VideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    bool Initialize(const char* file_name);
    void Uninitialize();

    // 获取某时间点帧（会自动触发seek）
    std::shared_ptr<cv::Mat> GetFrame(int time_ms, int timeout_ms = 100);

private:
    void Cleanup();
    void DecodeThreadLoop();
    void DecodeNextFrames(bool do_seek = false);

private:
    AVFormatContext* format_context_ = nullptr;
    AVCodecContext* codec_context_ = nullptr;
    SwsContext* sws_context_ = nullptr;

    std::thread decode_thread_;
    std::mutex mutex_;
    std::condition_variable cv_frame_;

    int video_stream_index_ = -1;
    AVPixelFormat current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    bool is_running_ = false;

    // 帧缓存
    std::deque<VideoFrame> frame_buffer_;
    const size_t max_cache_frames_ = 150; // ~6秒

    // seek 控制
    bool request_seek_ = false;
    int64_t seek_target_us_ = 0;

    // 上次解码位置
    int64_t last_pts_us_ = 0;

    static constexpr int64_t kDropToleranceUs = 40000;  // 40ms
    static constexpr int64_t kSeekThresholdUs = 2000000; // 2秒
};
