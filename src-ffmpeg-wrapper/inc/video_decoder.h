#ifndef FRAME_DECODER_H_
#define FRAME_DECODER_H_

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <opencv2/opencv.hpp>

#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

struct VideoFrame {
    int64_t pts_us;                // 时间戳，单位微妙
    std::shared_ptr<cv::Mat> frame_data; // 数据共享指针
};

class VideoDecoder {

public:
    VideoDecoder();
    ~VideoDecoder();

    bool Initialize(const char* fileName);
    void Uninitialize();

    /**
     * @brief 阻塞获取最近 time_ms 的帧，超时返回空指针
     * 
     * @param time_ms 
     * @param timeout_ms 
     * @return std::shared_ptr<cv::Mat> 
     */
    std::shared_ptr<cv::Mat> GetFrame(int time_ms, int timeout_ms = 100);

private:
    void DecodeThreadLoop();
    void DecodeWindow(int64_t start_pts_us);
    void Cleanup();

private:
    std::thread decode_thread_;
    std::atomic<bool> is_running_{false};
    std::atomic<bool> need_decode_{false};

    int64_t target_pts_us_ = 0;;

    std::mutex mutex_;
    std::condition_variable cv_decode_;  // 唤醒解码
    std::condition_variable cv_frame_;   // 唤醒播放线程

    std::deque<VideoFrame> frame_buffer_;
    size_t max_cache_frames_ = 180;

    AVFormatContext* format_context_ = nullptr;
    AVCodecContext*  codec_context_ = nullptr;
    SwsContext*      sws_context_ = nullptr;
    AVPixelFormat current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    int video_stream_index_ = -1;

    int64_t current_window_start_us_ = 0;
    int64_t current_window_end_us_ = 0;

    static constexpr int64_t kDecodeWindowUs  = 3 * 1000 * 1000;
    static constexpr int64_t kDropToleranceUs = 100 * 1000;
};

#endif