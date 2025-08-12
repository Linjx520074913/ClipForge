#define FFMPEG_WRAPPER_DLL

#include "ffmpeg_wrapper.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <format>
using namespace std;

struct StreamInfo {
    int index = -1;
    string type;
    int codec_id = 0;
    string codec_name;
    int width;
    int height;
    double fps = 0.0;
    int sample_rate = 0;
    std::map<std::string, std::string> metadata;
    bool is_best = false;
};

struct AVMetadata {
    string file_path;
    double duration = 0.0;
    int bit_rate = 0;
    std::map<std::string, std::string> metadata;
    std::vector<StreamInfo> streams;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StreamInfo,
    index, type, codec_id, codec_name, width, height, fps, sample_rate, metadata, is_best)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AVMetadata,
    file_path, duration, bit_rate, metadata, streams)


const char* get_version()
{
    return av_version_info();
}

const char* get_av_meta_data(const char* file_path)
{
//     AVMetadata meta;
//     meta.file_path = file_path;
//     nlohmann::json j = meta;
//     std::string s = j.dump();
//     char* buffer = (char*)malloc(s.size() + 1);
//     std::memcpy(buffer, s.c_str(), s.size() + 1);

    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, file_path, nullptr, nullptr) < 0) {
        std::cerr << "avformat_open_input failed: " << file_path << std::endl;
        return nullptr;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "avformat_find_stream_info failed" << std::endl;
        avformat_close_input(&fmt_ctx);
        return nullptr;
    }

    AVMetadata meta;
    meta.file_path = file_path;
    meta.duration = (fmt_ctx->duration > 0) ? (fmt_ctx->duration / (double)AV_TIME_BASE) : 0.0;
    meta.bit_rate = fmt_ctx->bit_rate;

    // 文件级元数据
    AVDictionaryEntry* tag = nullptr;
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        meta.metadata[tag->key] = tag->value;
    }

    // 找主视频流 & 主音频流
    int best_video = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int best_audio = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // 遍历所有流
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* par = stream->codecpar;
        StreamInfo si;
        si.index = i;
        si.codec_id = par->codec_id;

        const AVCodec* codec = avcodec_find_decoder(par->codec_id);
        if (codec) si.codec_name = codec->name;

        switch (par->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                si.type = "video";
                si.width = par->width;
                si.height = par->height;
                si.fps = (stream->avg_frame_rate.num && stream->avg_frame_rate.den) ?
                        av_q2d(stream->avg_frame_rate) : 0.0;
                break;
            case AVMEDIA_TYPE_AUDIO:
                si.type = "audio";
                si.sample_rate = par->sample_rate;
                break;
            default:
                si.type = "other";
                break;
        }

        // 流级元数据
        AVDictionaryEntry* st_tag = nullptr;
        while ((st_tag = av_dict_get(stream->metadata, "", st_tag, AV_DICT_IGNORE_SUFFIX))) {
            si.metadata[st_tag->key] = st_tag->value;
        }

        // 标记最佳流
        if ((int)i == best_video || (int)i == best_audio) {
            si.is_best = true;
        }

        meta.streams.push_back(std::move(si));
    }

    avformat_close_input(&fmt_ctx);

    // JSON 转字符串
    nlohmann::json j = meta;
    std::string s = j.dump();
    char* buffer = (char*)malloc(s.size() + 1);
    memcpy(buffer, s.c_str(), s.size() + 1);
    return buffer;
}

void free_av_meta_data(const char* ptr)
{
    if(ptr) {
        free((void*)ptr);
    }
}

const char* get_json_string()
{
    nlohmann::json j;
    j["name"] = "ClipForge";
    j["version"] = "1.0.0";

    std::string s = j.dump();

    char* buffer = (char*)malloc(s.size() + 1);
    std::memcpy(buffer, s.c_str(), s.size() + 1);
    return buffer;
}

void get_media_meta(const char* filePath)
{
    AVFormatContext *fmt_ctx = NULL;
    int ret;
    
    // 打开视频文件
    ret = avformat_open_input(&fmt_ctx, filePath, NULL, NULL);
    if (ret != 0) {
        printf("无法打开视频文件\n");
        return;
    }
    
    // 获取视频文件中每个流的详细信息
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        printf("无法获取视频流信息\n");
        return;
    }
    
    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *st = fmt_ctx->streams[i];
        AVCodecParameters *par = st->codecpar;
    
        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            printf("Video Stream %d: %dx%d, codec id: %d, fps: %.2f\n",
                   i, par->width, par->height, par->codec_id,
                   av_q2d(st->avg_frame_rate));
        }
        else if (par->codec_type == AVMEDIA_TYPE_AUDIO) {
            printf("Audio Stream %d: sample_rate=%d, codec id: %d\n",
                   i, par->sample_rate, par->codec_id);
        }
        else {
            printf("Other Stream %d: codec id %d\n", i, par->codec_id);
        }
    }
    
    // 获取元数据（metadata）
    AVDictionaryEntry *tag = NULL;
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        printf("%s=%s\n", tag->key, tag->value);
    }
    
    // 关闭输入文件
    avformat_close_input(&fmt_ctx);
}


AVFormatContext *fmt_ctx = nullptr;
AVCodecContext *video_dec_ctx = nullptr;
int videoStreamIdx = -1;

static AVPacket *pkt = nullptr;
static AVFrame *frame = nullptr;
static AVFrame *rgb_frame = nullptr;
static struct SwsContext *sws_ctx = nullptr;
static uint8_t *rgb_buffer = nullptr;

// 释放资源函数
void cleanup() {
    if (rgb_buffer) {
        av_free(rgb_buffer);
        rgb_buffer = nullptr;
    }
    if (rgb_frame) {
        av_frame_free(&rgb_frame);
        rgb_frame = nullptr;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (pkt) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    if (video_dec_ctx) {
        avcodec_free_context(&video_dec_ctx);
        video_dec_ctx = nullptr;
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
}

void open_video(const char* filePath)
{
    int ret = 0;

    // 初始化对象
    pkt = av_packet_alloc();
    frame = av_frame_alloc();
    rgb_frame = av_frame_alloc();

    if (!pkt || !frame || !rgb_frame) {
        cerr << "Failed to allocate AVPacket or AVFrame" << endl;
        cleanup();
        return;
    }

    if ((ret = avformat_open_input(&fmt_ctx, filePath, NULL, NULL)) < 0) {
        cerr << "avformat_open_input error: " << filePath << endl;
        cleanup();
        return;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        cerr << "avformat_find_stream_info error: " << filePath << endl;
        cleanup();
        return;
    }

    cout << "Total ms = " << (fmt_ctx->duration / (AV_TIME_BASE / 1000)) << endl;

    videoStreamIdx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoStreamIdx < 0) {
        cerr << "Could not find video stream" << endl;
        cleanup();
        return;
    }

    AVStream *videoStream = fmt_ctx->streams[videoStreamIdx];
    const AVCodec *decoder = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!decoder) {
        cerr << "Codec not found" << endl;
        cleanup();
        return;
    }

    video_dec_ctx = avcodec_alloc_context3(decoder);
    if (!video_dec_ctx) {
        cerr << "Could not allocate codec context" << endl;
        cleanup();
        return;
    }

    if ((ret = avcodec_parameters_to_context(video_dec_ctx, videoStream->codecpar)) < 0) {
        cerr << "avcodec_parameters_to_context failed" << endl;
        cleanup();
        return;
    }

    if ((ret = avcodec_open2(video_dec_ctx, decoder, NULL)) < 0) {
        cerr << "Could not open codec" << endl;
        cleanup();
        return;
    }

    sws_ctx = sws_getContext(
        video_dec_ctx->width,
        video_dec_ctx->height,
        video_dec_ctx->pix_fmt,
        video_dec_ctx->width,
        video_dec_ctx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        cerr << "Could not initialize sws context" << endl;
        cleanup();
        return;
    }

    int rgb_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, video_dec_ctx->width, video_dec_ctx->height, 1);
    rgb_buffer = (uint8_t*)av_malloc(rgb_buffer_size);
    if (!rgb_buffer) {
        cerr << "Could not allocate RGB buffer" << endl;
        cleanup();
        return;
    }

    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24,
                         video_dec_ctx->width, video_dec_ctx->height, 1);
    rgb_frame->width = video_dec_ctx->width;
    rgb_frame->height = video_dec_ctx->height;
    rgb_frame->format = AV_PIX_FMT_RGB24;

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == videoStreamIdx) {
            if (avcodec_send_packet(video_dec_ctx, pkt) == 0) {
                while (avcodec_receive_frame(video_dec_ctx, frame) == 0) {
                    cout << "Decoded frame: pts=" << frame->pts 
                         << " w=" << frame->width 
                         << " h=" << frame->height << endl;

                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                              rgb_frame->data, rgb_frame->linesize);

                    cv::Mat mat(rgb_frame->height, rgb_frame->width, CV_8UC3, rgb_frame->data[0], rgb_frame->linesize[0]);
                    cv::imshow("Video", mat);
                    if (cv::waitKey(1) == 27) { // ESC键退出
                        cleanup();
                        return;
                    }
                    av_frame_unref(frame);
                }
            }
        }
        av_packet_unref(pkt);
    }

    cout << "Decoding finished!" << endl;
    cleanup();
}

void get_frame(int millisecond)
{

}

void close_video()
{

}