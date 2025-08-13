#include "clipforge_decoder.h"

#include <opencv2/opencv.hpp>

using namespace std;

const char* CFDecoder::get_version()
{
    return av_version_info();
}

const char* CFDecoder::get_av_meta_data(const char* file_path)
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

    CFMetadata meta;
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
        CFStreamInfo si;
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

void CFDecoder::free_av_meta_data(const char* ptr)
{
    if(ptr) {
        free((void*)ptr);
    }
}

void CFDecoder::open_video(const char* file_path)
{
    if(avformat_open_input(&fmt_ctx_, file_path, nullptr, nullptr) < 0) {
        cerr << "Failed to open input file : " << file_path << endl;
        return;
    }

    if(avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
        cerr << "Failed to find stream info" << endl;
        cleanup();
        return;
    }

    best_video_stream_idx_ = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(best_video_stream_idx_ < 0) {
        cerr << "No video stream found" << endl;
        cleanup();
        return;
    }

    AVStream* stream = fmt_ctx_->streams[best_video_stream_idx_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!decoder) {
        cerr << "Decoder not found" << endl;
        cleanup();
        return;
    }

    codec_ctx_ = avcodec_alloc_context3(decoder);
    if(!codec_ctx_) {
        cerr << "Failed to allocate codec context" << endl;
        cleanup();
        return;
    }

    if(avcodec_parameters_to_context(codec_ctx_, stream->codecpar) < 0) {
        cerr << "Failed to copy codec paramters" << endl;
        cleanup();
        return;
    }

    if(avcodec_open2(codec_ctx_, decoder, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        cleanup();
        return;
    }

    sws_ctx_ = nullptr;

    decode_thread_ = std::thread(&CFDecoder::decode_loop, this);
}

void CFDecoder::decode_loop()
{
    AVPacket* pkt       = av_packet_alloc();
    AVFrame*  frame_raw = av_frame_alloc();

    sws_ctx_ = nullptr;

    auto decode_and_show = [&](AVPacket* p){
        if(avcodec_send_packet(codec_ctx_, p) < 0) return;

        while(avcodec_receive_frame(codec_ctx_, frame_raw) == 0) {
            if (!sws_ctx_) {
                sws_ctx_ = sws_getContext(
                    frame_raw->width, frame_raw->height, static_cast<AVPixelFormat>(frame_raw->format),
                    frame_raw->width, frame_raw->height, AV_PIX_FMT_BGRA,
                    SWS_BILINEAR, nullptr, nullptr, nullptr
                );
                if (!sws_ctx_) {
                    cerr << "Failed to create SwsContext" << endl;
                    exit(-1);
                }
            }

            int width = frame_raw->width;
            int height = frame_raw->height;

            cv::Mat mat(height, width, CV_8UC4);

            uint8_t* dst_data[4] = { mat.data, nullptr, nullptr, nullptr };
            int dst_linesize[4] = { static_cast<int>(mat.step[0]), 0, 0, 0 };

            sws_scale(sws_ctx_, frame_raw->data, frame_raw->linesize, 0, height, dst_data, dst_linesize);

            // cv::imshow("CFDecoder", mat);
            // if (cv::waitKey(1) == 'q') exit(0);
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [&]{ return frame_queue_.size() < max_queue_size_ || stop_requested_; });
                if(stop_requested_) return;

                frame_queue_.push(std::move(mat));
            }

            queue_cv_.notify_all();
        }
    };

    while(av_read_frame(fmt_ctx_, pkt) >= 0) {
        if(pkt->stream_index == best_video_stream_idx_) {
            decode_and_show(pkt);
        }
        av_packet_unref(pkt);
    }

    av_frame_free(&frame_raw);
    av_packet_free(&pkt);
}

// void CFDecoder::get_frame(cv::Mat& frame)
// {
//     std::unique_lock<std::mutex> lock(queue_mutex_);
//     queue_cv_.wait(lock, [&]{ return !frame_queue_.empty() || stop_requested_; });

//     if(frame_queue_.empty() && stop_requested_) {
//         return;
//     }

//     frame = std::move(frame_queue_.front());
//     frame_queue_.pop();
//     queue_cv_.notify_all();
// }

CFFrame* CFDecoder::get_frame()
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_cv_.wait(lock, [&]{ return !frame_queue_.empty() || stop_requested_; });

    if(frame_queue_.empty() && stop_requested_) {
        return nullptr;
    }

    cv::Mat cur = std::move(frame_queue_.front());
    frame_queue_.pop();
    queue_cv_.notify_all();

    CFFrame* frame = new CFFrame();
    frame->width = cur.cols;
    frame->height= cur.rows;
    size_t data_size = cur.cols * cur.rows * cur.channels();
    frame->length = data_size;
    cout << "FFFFFFFFFF " << data_size << endl;
    frame->data = new uint8_t[data_size];
    memcpy(frame->data, cur.data, data_size);

    return frame;
}

void CFDecoder::free_frame(const CFFrame* ptr)
{
    if(ptr) {
        delete[] ptr->data;
        delete ptr;
    }
}

void CFDecoder::close_video()
{
    cleanup();
}

void CFDecoder::cleanup()
{
    if(codec_ctx_) {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }
    if(fmt_ctx_) {
        avformat_close_input(&fmt_ctx_);
        fmt_ctx_ = nullptr;
    }
    if(sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }
    if(decode_thread_.joinable()) {
        decode_thread_.join();
    }
}
