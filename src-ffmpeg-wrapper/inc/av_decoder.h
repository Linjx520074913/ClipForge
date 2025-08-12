#ifndef AV_DECODER_H_
#define AV_DECODER_H_

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
#include <nlohmann/json.hpp>

// 表示单个媒体流的信息（视频、音频或其他类型）
struct AVStreamInfo {
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
struct AVMetadata {
    std::string file_path;                      // 媒体文件的路径
    double duration = 0.0;                      // 媒体文件总时长，单位秒
    int bit_rate = 0;                           // 媒体文件整体比特率，单位比特每秒
    std::map<std::string, std::string> entries; // 媒体文件级别的元数据键值对集合
    std::vector<AVStreamInfo> streams;          // 媒体文件级别的元数据键值对集合
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AVStreamInfo,
    index, type, codec_id, codec_name, width, height, fps, sample_rate, entries, is_best)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AVMetadata,
    file_path, duration, bit_rate, entries, streams)


class AVDecoder
{

public:
    AVDecoder() = default;
    ~AVDecoder() = default;

public:
    static const char* get_version();
    static const char* get_av_meta_data(const char* file_path);
    static void free_av_meta_data(const char* ptr);

private:

};

#endif