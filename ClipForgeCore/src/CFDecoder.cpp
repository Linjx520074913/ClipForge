#include "CFDecoder.h"
#include <iostream>

using namespace std;

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

std::shared_ptr<CFFrame> CFDecoder::get_frame(double timestamp)
{
    return frame_cache_.get_frame(timestamp);
}

void CFDecoder::decode_loop()
{
    AVPacket* pkt   = av_packet_alloc();
    AVFrame*  frame = av_frame_alloc();

    sws_ctx_ = nullptr;

    auto decode_and_show = [&](AVPacket* p){
        if(avcodec_send_packet(codec_ctx_, p) < 0) return;

        while(avcodec_receive_frame(codec_ctx_, frame) == 0) {
            if (!sws_ctx_) {
                sws_ctx_ = sws_getContext(
                    frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
                    frame->width, frame->height, AV_PIX_FMT_BGRA,
                    SWS_BILINEAR, nullptr, nullptr, nullptr
                );
                if (!sws_ctx_) {
                    cerr << "Failed to create SwsContext" << endl;
                    exit(-1);
                }
            }

            auto cfframe = std::make_shared<CFFrame>();
            cfframe->width = frame->width;
            cfframe->height= frame->height;
            cfframe->channels = 4;
            cfframe->length = frame->width * frame->height * 4;
            cfframe->data = std::make_shared<uint8_t[]>(cfframe->length);

            uint8_t* dst_data[4] = { cfframe->data.get(), nullptr, nullptr, nullptr };
            int dst_linesize[4] = { static_cast<int>(frame->width * 4), 0, 0, 0 };

            sws_scale(sws_ctx_, frame->data, frame->linesize, 0, frame->height, dst_data, dst_linesize);

            double timestamp = frame->pts * av_q2d(fmt_ctx_->streams[best_video_stream_idx_]->time_base) * 1000;
            cfframe->timestamp = timestamp;
            frame_cache_.push(timestamp, cfframe);
        }
    };

    while(av_read_frame(fmt_ctx_, pkt) >= 0) {
        if(pkt->stream_index == best_video_stream_idx_) {
            decode_and_show(pkt);
        }
        av_packet_unref(pkt);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

