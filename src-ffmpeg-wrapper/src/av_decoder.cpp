#include "av_decoder.h"

using namespace std;

const char* AVDecoder::get_version()
{
    return av_version_info();
}
    
const char* AVDecoder::get_av_meta_data(const char* file_path)
{
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
        meta.entries[tag->key] = tag->value;
    }

    // 找主视频流 & 主音频流
    int best_video = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int best_audio = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

    // 遍历所有流
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* par = stream->codecpar;
        AVStreamInfo si;
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
            si.entries[st_tag->key] = st_tag->value;
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

void AVDecoder::free_av_meta_data(const char* ptr)
{
    if(ptr) {
        free((void*)ptr);
    }
}