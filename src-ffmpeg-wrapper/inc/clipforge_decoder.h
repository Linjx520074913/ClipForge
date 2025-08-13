#ifndef CLIP_FORGE_DECODER_H_
#define CLIP_FORGE_DECODER_H_

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <memory>
#include <condition_variable>
#include <nlohmann/json.hpp>

#include <opencv2/opencv.hpp>


// 项目叫 ClipForge，采用 CF 为前缀

// 表示单个媒体流的信息（视频、音频或其他类型）
struct CFStreamInfo {
    int index = -1;                             // 媒体流的索引号
    std::string type;                           // 流的类型，如 "video"（视频）、"audio"（音频）、"other"（其他）
    int codec_id = 0;                           // 流所使用的编码格式 ID
    std::string codec_name;                     // 编码器名称，便于识别
    int width;                                  // 视频流的宽度（像素），仅视频流有效
    int height;                                 // 视频流的高度（像素），仅视频流有效
    double fps = 0.0;                           // 视频帧率（帧每秒），仅视频流有效
    int sample_rate = 0;                        // 音频采样率（赫兹），仅音频流有效
    std::map<std::string, std::string> entries; // 此流的元数据键值对集合
    bool is_best = false;                       // 是否为“最佳流”，如主视频流或主音频流
};

// 表示整个媒体文件的元数据信息
struct CFMetadata {
    std::string file_path;                      // 媒体文件的路径
    double duration = 0.0;                      // 媒体文件总时长，单位秒
    int bit_rate = 0;                           // 媒体文件整体比特率，单位比特每秒
    std::map<std::string, std::string> entries; // 媒体文件级别的元数据键值对集合
    std::vector<CFStreamInfo> streams;          // 媒体文件级别的元数据键值对集合
};

struct CFFrame {
    int width;
    int height;
    int length;
    uint8_t* data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CFStreamInfo,
    index, type, codec_id, codec_name, width, height, fps, sample_rate, entries, is_best)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CFMetadata,
    file_path, duration, bit_rate, entries, streams)


class CFDecoder
{

public:
    CFDecoder() = default;
    ~CFDecoder() = default;

    void open_video(const char* file_path);
    void close_video();

    // void get_frame(int millisecond);
    // void get_frame(cv::Mat& frame);

    CFFrame* get_frame();
    void free_frame(const CFFrame* ptr);

    void decode_loop();

    /**
     * @brief 释放资源
     * 
     */
    void cleanup();

public:
    static const char* get_version();
    static const char* get_av_meta_data(const char* file_path);
    static void free_av_meta_data(const char* ptr);

private:
    std::thread decode_thread_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    std::queue<cv::Mat> frame_queue_;
    size_t max_queue_size_ = 100;
    std::atomic<bool> stop_requested_ { false };

    AVFormatContext* fmt_ctx_ = nullptr;
    AVCodecContext*  codec_ctx_ = nullptr;
    SwsContext*      sws_ctx_ = nullptr;

    int best_video_stream_idx_ = -1;
};

#endif