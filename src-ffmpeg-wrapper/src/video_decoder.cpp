#include "video_decoder.h"
#include <iostream>

VideoDecoder::VideoDecoder() = default;

VideoDecoder::~VideoDecoder() {
    Uninitialize();
}

bool VideoDecoder::Initialize(const char* file_path) {
    if (is_running_) return false;

    if (avformat_open_input(&format_ctx_, file_path, nullptr, nullptr) < 0) {
        std::cerr << "Failed to open input file: " << file_path << std::endl;
        return false;
    }

    if (avformat_find_stream_info(format_ctx_, nullptr) < 0) {
        std::cerr << "Failed to find stream info" << std::endl;
        Cleanup();
        return false;
    }

    video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index_ < 0) {
        std::cerr << "No video stream found" << std::endl;
        Cleanup();
        return false;
    }

    AVStream* stream = format_ctx_->streams[video_stream_index_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        std::cerr << "Decoder not found" << std::endl;
        Cleanup();
        return false;
    }

    codec_ctx_ = avcodec_alloc_context3(decoder);
    if (!codec_ctx_) {
        std::cerr << "Failed to allocate codec context" << std::endl;
        Cleanup();
        return false;
    }

    if (avcodec_parameters_to_context(codec_ctx_, stream->codecpar) < 0) {
        std::cerr << "Failed to copy codec parameters" << std::endl;
        Cleanup();
        return false;
    }

    if (avcodec_open2(codec_ctx_, decoder, nullptr) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        Cleanup();
        return false;
    }

    sws_ctx_ = nullptr;
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    current_window_start_us_ = 0;
    current_window_end_us_ = 0;

    is_running_ = true;
    need_decode_ = true;
    target_pts_us_ = 0;

    decode_thread_ = std::thread(&VideoDecoder::DecodeThreadLoop, this);
    return true;
}

void VideoDecoder::Uninitialize() {
    if (!is_running_) return;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        is_running_ = false;
        cv_decode_.notify_all();
        cv_frame_.notify_all();
    }

    if (decode_thread_.joinable()) decode_thread_.join();
    Cleanup();
}

void VideoDecoder::Cleanup() {
    if (codec_ctx_) {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }
    if (format_ctx_) {
        avformat_close_input(&format_ctx_);
        format_ctx_ = nullptr;
    }
    if (sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }

    frame_buffer_.clear();
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;
    current_window_start_us_ = 0;
    current_window_end_us_ = 0;
}

void VideoDecoder::DecodeThreadLoop() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_decode_.wait(lock, [this]() { return need_decode_ || !is_running_; });

        if (!is_running_) break;

        need_decode_ = false;
        frame_buffer_.clear();

        int64_t decode_start_pts = current_window_start_us_;
        lock.unlock();

        DecodeWindow(decode_start_pts);
    }
}

void VideoDecoder::DecodeWindow(int64_t start_pts_us) {
    current_window_start_us_ = start_pts_us;
    current_window_end_us_ = start_pts_us + kDecodeWindowUs;

    int64_t seek_target = av_rescale_q(current_window_start_us_, AVRational{1, 1000000}, format_ctx_->streams[video_stream_index_]->time_base);
    if (av_seek_frame(format_ctx_, video_stream_index_, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        std::cerr << "Warning: av_seek_frame failed" << std::endl;
    }
    avcodec_flush_buffers(codec_ctx_);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while (is_running_) {
        if (av_read_frame(format_ctx_, pkt) < 0) break;

        if (pkt->stream_index != video_stream_index_) {
            av_packet_unref(pkt);
            continue;
        }

        if (avcodec_send_packet(codec_ctx_, pkt) < 0) {
            av_packet_unref(pkt);
            continue;
        }

        av_packet_unref(pkt);

        while (avcodec_receive_frame(codec_ctx_, frame) == 0) {
            int64_t pts_us = av_rescale_q(frame->pts, format_ctx_->streams[video_stream_index_]->time_base, AVRational{1, 1000000});
            if (pts_us < current_window_start_us_) continue;
            if (pts_us > current_window_end_us_) goto decode_end;

            AVPixelFormat src_pix_fmt = static_cast<AVPixelFormat>(frame->format);
            if (!sws_ctx_ || src_pix_fmt != current_src_pix_fmt_) {
                if (sws_ctx_) sws_freeContext(sws_ctx_);
                sws_ctx_ = sws_getContext(
                    frame->width, frame->height, src_pix_fmt,
                    frame->width, frame->height, AV_PIX_FMT_BGR24,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);
                if (!sws_ctx_) {
                    std::cerr << "Failed to create sws context" << std::endl;
                    goto decode_end;
                }
                current_src_pix_fmt_ = src_pix_fmt;
            }

            auto mat = std::make_shared<cv::Mat>(frame->height, frame->width, CV_8UC3);
            uint8_t* dst[4] = { mat->data, nullptr, nullptr, nullptr };
            int dst_linesize[4] = { static_cast<int>(mat->step), 0, 0, 0 };
            sws_scale(sws_ctx_, frame->data, frame->linesize, 0, frame->height, dst, dst_linesize);

            {
                std::lock_guard<std::mutex> lock(mutex_);
                frame_buffer_.emplace_back(VideoFrame{ pts_us, mat });
                if (frame_buffer_.size() > max_cache_frames_)
                    frame_buffer_.pop_front();
            }
            cv_frame_.notify_all();

            // 控制缓存大小，满了先睡眠，减少 CPU 占用
            {
                std::unique_lock<std::mutex> lock(mutex_);
                while (frame_buffer_.size() >= max_cache_frames_ && is_running_) {
                    cv_decode_.wait_for(lock, std::chrono::milliseconds(10));
                }
            }
        }
    }

decode_end:
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

std::shared_ptr<cv::Mat> VideoDecoder::GetFrame(int time_ms, int timeout_ms) {
    int64_t requested_pts_us = static_cast<int64_t>(time_ms) * 1000;

    std::unique_lock<std::mutex> lock(mutex_);

    // 判断是否需要跳转窗口，滑动缓存
    bool need_slide = false;
    if (frame_buffer_.empty()) {
        need_slide = true;
    }
    else if (requested_pts_us < current_window_start_us_ || requested_pts_us > current_window_end_us_) {
        need_slide = true;
    }

    if (need_slide) {
        current_window_start_us_ = requested_pts_us;
        current_window_end_us_ = requested_pts_us + kDecodeWindowUs;

        target_pts_us_ = current_window_start_us_;
        need_decode_ = true;
        cv_decode_.notify_one();

        frame_buffer_.clear();
    }

    // 等待满足条件的帧
    cv_frame_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this, requested_pts_us]() {
        if (!is_running_ || frame_buffer_.empty()) return false;
        for (auto& f : frame_buffer_) {
            if (std::abs(f.pts_us - requested_pts_us) <= kDropToleranceUs) return true;
        }
        return false;
    });

    if (frame_buffer_.empty()) return nullptr;

    // 丢弃过时帧，保持最新帧可用
    while (frame_buffer_.size() > 1 && frame_buffer_.front().pts_us + kDropToleranceUs < requested_pts_us) {
        frame_buffer_.pop_front();
    }

    // 找最近帧返回
    std::shared_ptr<cv::Mat> best_frame = nullptr;
    int64_t min_diff = INT64_MAX;
    for (auto& f : frame_buffer_) {
        int64_t diff = std::abs(f.pts_us - requested_pts_us);
        if (diff < min_diff) {
            min_diff = diff;
            best_frame = f.frame_data;
        }
    }

    return best_frame;
}
