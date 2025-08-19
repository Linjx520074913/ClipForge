#include "CFDecoder.h"
#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

CFDecoder::CFDecoder() {}
CFDecoder::~CFDecoder() { close_video(); }

bool CFDecoder::open_video(const char* file_path) {
    close_video();

    if (avformat_open_input(&fmt_ctx_, file_path, nullptr, nullptr) < 0) { return false; }
    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) { return false; }

    best_video_stream_idx_ = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (best_video_stream_idx_ < 0) { return false; }

    AVStream* stream = fmt_ctx_->streams[best_video_stream_idx_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) { return false; }

    codec_ctx_ = avcodec_alloc_context3(decoder);
    if (!codec_ctx_) { return false; }
    if (avcodec_parameters_to_context(codec_ctx_, stream->codecpar) < 0) { return false; }
    if (avcodec_open2(codec_ctx_, decoder, nullptr) < 0) { return false; }

    width_ = codec_ctx_->width;
    height_ = codec_ctx_->height;

    AVRational fr = stream->avg_frame_rate.num ? stream->avg_frame_rate : stream->r_frame_rate;
    fps_ = (fr.num && fr.den) ? av_q2d(fr) : 30.0;
    frame_interval_ms_ = (fps_ > 1e-6) ? (1000.0 / fps_) : 33.333;

    sws_ctx_ = nullptr;
    eof_ = false;
    running_ = true;
    decode_thread_ = thread(&CFDecoder::decode_loop, this);

    return true;
}

void CFDecoder::close_video() { cleanup(); }

void CFDecoder::cleanup() {
    running_ = false;
    decode_cv_.notify_all();
    if (decode_thread_.joinable()) decode_thread_.join();

    lock_guard<mutex> lk(display_mutex_);
    display_queue_.clear();
    last_frame_ = nullptr;

    if (codec_ctx_) { avcodec_free_context(&codec_ctx_); codec_ctx_ = nullptr; }
    if (fmt_ctx_) { avformat_close_input(&fmt_ctx_); fmt_ctx_ = nullptr; }
    if (sws_ctx_) { sws_freeContext(sws_ctx_); sws_ctx_ = nullptr; }

    best_video_stream_idx_ = -1; width_ = height_ = 0; fps_ = 0.0;
}

void CFDecoder::push_to_display_queue(shared_ptr<CFFrame> frame) {
    lock_guard<mutex> lk(display_mutex_);
    display_queue_.push_back({ frame->timestamp, frame });
    last_frame_ = frame;

    while (!display_queue_.empty() &&
           display_queue_.back().timestamp - display_queue_.front().timestamp > hard_cap_ms_) {
        display_queue_.pop_front();
    }
}

std::shared_ptr<CFFrame> CFDecoder::get_frame(double timestamp_ms) {
    std::shared_ptr<CFFrame> best = nullptr;
    double min_diff = numeric_limits<double>::max();

    lock_guard<mutex> lk(display_mutex_);
    if (display_queue_.empty()) return last_frame_;

    for (auto& f : display_queue_) {
        double diff = abs(f.timestamp - timestamp_ms);
        if (diff < min_diff) { min_diff = f.frame ? f.frame->timestamp : numeric_limits<double>::max(); best = f.frame; }
    }

    if (!best) best = last_frame_;
    return best;
}

void CFDecoder::decode_loop() {
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVStream* stream = fmt_ctx_->streams[best_video_stream_idx_];

    while (running_) {
        // 等待缓存低于 high_watermark_ms_ 或停止信号
        {
            unique_lock<mutex> lk(display_mutex_);
            decode_cv_.wait_for(lk, chrono::milliseconds(2), [this]() {
                if (!running_) return true;
                if (display_queue_.empty()) return true;
                double cache_time = display_queue_.back().timestamp - display_queue_.front().timestamp;
                return cache_time < high_watermark_ms_;
            });
        }

        if (!running_) break;

        int ret = av_read_frame(fmt_ctx_, pkt);
        if (ret < 0) {
            avcodec_send_packet(codec_ctx_, nullptr);
            while (avcodec_receive_frame(codec_ctx_, frame) == 0) {
                auto f = make_shared<CFFrame>();
                f->width = frame->width;
                f->height = frame->height;
                f->channels = 4;
                f->length = f->width * f->height * 4;
                f->data = make_shared<uint8_t[]>(f->length);
                f->timestamp = pts_to_ms(frame->pts, stream->time_base);

                if (!sws_ctx_) {
                    sws_ctx_ = sws_getContext(f->width, f->height,
                                              static_cast<AVPixelFormat>(frame->format),
                                              f->width, f->height,
                                              AV_PIX_FMT_BGRA,
                                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                }

                uint8_t* dst[4] = { f->data.get(), nullptr, nullptr, nullptr };
                int linesize[4] = { f->width * 4, 0, 0, 0 };
                sws_scale(sws_ctx_, frame->data, frame->linesize, 0, f->height, dst, linesize);

                push_to_display_queue(f);
            }
            eof_ = true;
            break;
        }

        if (pkt->stream_index == best_video_stream_idx_) {
            ret = avcodec_send_packet(codec_ctx_, pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                av_packet_unref(pkt);
                continue;
            }

            while ((ret = avcodec_receive_frame(codec_ctx_, frame)) == 0) {
                auto f = make_shared<CFFrame>();
                f->width = frame->width;
                f->height = frame->height;
                f->channels = 4;
                f->length = f->width * f->height * 4;
                f->data = make_shared<uint8_t[]>(f->length);
                f->timestamp = pts_to_ms(frame->pts, stream->time_base);

                if (!sws_ctx_) {
                    sws_ctx_ = sws_getContext(f->width, f->height,
                                              static_cast<AVPixelFormat>(frame->format),
                                              f->width, f->height,
                                              AV_PIX_FMT_BGRA,
                                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                }

                uint8_t* dst[4] = { f->data.get(), nullptr, nullptr, nullptr };
                int linesize[4] = { f->width * 4, 0, 0, 0 };
                sws_scale(sws_ctx_, frame->data, frame->linesize, 0, f->height, dst, linesize);

                push_to_display_queue(f);
            }
        }

        av_packet_unref(pkt);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}
