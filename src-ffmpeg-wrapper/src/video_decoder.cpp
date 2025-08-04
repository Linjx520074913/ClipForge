#include "video_decoder.h"
#include <iostream>

using namespace std;

VideoDecoder::VideoDecoder() = default;

VideoDecoder::~VideoDecoder()
{
    Uninitialize();
}

bool VideoDecoder::Initialize(const char* file_name)
{
    if(is_running_) return false;

    if(avformat_open_input(&format_context_, file_name, nullptr, nullptr) < 0) {
        cerr << "Failed to open input file : " << file_name << endl;
        return false;
    }

    if(avformat_find_stream_info(format_context_, nullptr) < 0) {
        cerr << "Failed to find stream information" << endl;
        Cleanup();
        return false;
    }

    video_stream_index_ = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(video_stream_index_ < 0) {
        cerr << "No video stream found" << endl;
        Cleanup();
        return false;
    }

    AVStream* stream = format_context_->streams[video_stream_index_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!decoder) {
        cerr << "Codec not found" << endl;
        Cleanup();
        return false;
    }

    codec_context_ = avcodec_alloc_context3(decoder);
    if(!codec_context_) {
        cerr << "Could not allocate codec context" << endl;
        Cleanup();
        return false;
    }

    if(avcodec_parameters_to_context(codec_context_, stream->codecpar) < 0) {
        cerr << "Failed to copy codec parameters" << endl;
        Cleanup();
        return false;
    }

    if(avcodec_open2(codec_context_, decoder, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        Cleanup();
        return false;
    }

    sws_context_ = nullptr;
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    // 初始化缓存窗口起止时间
    current_window_start_us_ = 0;
    current_window_end_us_ = 0;

    is_running_ = true;
    need_decode_ = true;
    target_pts_us_ = 0;
    decode_thread_ = std::thread(&VideoDecoder::DecodeThreadLoop, this);

    return true;
}

void VideoDecoder::Uninitialize()
{
    if(!is_running_) return;

    is_running_ = false;
    cv_decode_.notify_all();
    cv_frame_.notify_all();

    if(decode_thread_.joinable()) {
        decode_thread_.join();
    }
    Cleanup();
}

void VideoDecoder::Cleanup()
{
    if(codec_context_) {
        avcodec_free_context(&codec_context_);
        codec_context_ = nullptr;
    }
    if(format_context_) {
        avformat_close_input(&format_context_);
        format_context_ = nullptr;
    }
    if(sws_context_) {
        sws_freeContext(sws_context_);
        sws_context_ = nullptr;
    }
    frame_buffer_.clear();
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;
    current_window_start_us_ = 0;
    current_window_end_us_ = 0;
}

void VideoDecoder::DecodeThreadLoop()
{
    while(is_running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_decode_.wait(lock, [this]() { return need_decode_ || !is_running_; });
        if(!is_running_) break;

        need_decode_ = false;
        frame_buffer_.clear();

        lock.unlock();

        DecodeWindow(current_window_start_us_);
    }
}

void VideoDecoder::DecodeWindow(int64_t start_pts_us)
{
    current_window_start_us_ = start_pts_us;
    current_window_end_us_ = start_pts_us + kDecodeWindowUs;

    int64_t seek_target = av_rescale_q(current_window_start_us_, AVRational{1, 1000000}, format_context_->streams[video_stream_index_]->time_base);
    if(av_seek_frame(format_context_, video_stream_index_, seek_target, AVSEEK_FLAG_BACKWARD) < 0) {
        cerr << "Warning: av_seek_frame failed" << endl;
    }
    avcodec_flush_buffers(codec_context_);

    AVPacket* pkt = av_packet_alloc();
    AVFrame*  frame = av_frame_alloc();

    while(av_read_frame(format_context_, pkt) >= 0) {
        if(!is_running_) break;

        if(pkt->stream_index != video_stream_index_) {
            av_packet_unref(pkt);
            continue;
        }

        avcodec_send_packet(codec_context_, pkt);
        while(avcodec_receive_frame(codec_context_, frame) == 0) {
            int64_t pts_us = av_rescale_q(frame->pts, format_context_->streams[video_stream_index_]->time_base, AVRational{1, 1000000});
            if(pts_us < current_window_start_us_) continue;
            if(pts_us > current_window_end_us_) {
                av_packet_unref(pkt);
                goto decode_end;
            }

            AVPixelFormat src_pix_fmt = static_cast<AVPixelFormat>(frame->format);
            if(!sws_context_ || src_pix_fmt != current_src_pix_fmt_) {
                if(sws_context_) {
                    sws_freeContext(sws_context_);
                }
                sws_context_ = sws_getContext(
                    frame->width, frame->height, src_pix_fmt,
                    frame->width, frame->height, AV_PIX_FMT_BGR24,
                    SWS_BILINEAR, nullptr, nullptr, nullptr
                );
                if(!sws_context_) {
                    std::cerr << "Failed to create sws context" << std::endl;
                    break;
                }
                current_src_pix_fmt_ = src_pix_fmt;
            }

            auto mat = std::make_shared<cv::Mat>(frame->height, frame->width, CV_8UC3);
            uint8_t* dst[4] = { mat->data, nullptr, nullptr, nullptr };
            int dst_linesize[4] = { static_cast<int>(mat->step), 0, 0, 0 };
            sws_scale(sws_context_, frame->data, frame->linesize, 0, frame->height, dst, dst_linesize);

            {
                std::lock_guard<std::mutex> lock(mutex_);
                frame_buffer_.emplace_back(VideoFrame{pts_us, mat});
                if(frame_buffer_.size() > max_cache_frames_) {
                    frame_buffer_.pop_front();
                }
            }
            cv_frame_.notify_all();
        }
        av_packet_unref(pkt);
    }

decode_end:
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

std::shared_ptr<cv::Mat> VideoDecoder::GetFrame(int time_ms, int timeout_ms)
{
    int64_t requested_pts_us = static_cast<int64_t>(time_ms) * 1000;

    std::unique_lock<std::mutex> lock(mutex_);

    // 判断请求时间是否在当前缓存窗口外，若是则滑动窗口
    bool need_slide = false;
    if(frame_buffer_.empty()) {
        need_slide = true;
    } else if(requested_pts_us < current_window_start_us_ || requested_pts_us > current_window_end_us_) {
        need_slide = true;
    }

    if(need_slide) {
        current_window_start_us_ = requested_pts_us;
        current_window_end_us_ = requested_pts_us + kDecodeWindowUs;

        target_pts_us_ = current_window_start_us_;
        need_decode_ = true;
        cv_decode_.notify_one();

        // 清空旧缓存，保证解码新窗口
        frame_buffer_.clear();
    }

    // 等待目标帧解码完成
    cv_frame_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this, requested_pts_us]() {
        if(!is_running_ || frame_buffer_.empty()) return false;
        for(auto& item : frame_buffer_) {
            if(std::abs(item.pts_us - requested_pts_us) <= kDropToleranceUs) return true;
        }
        return false;
    });

    if(frame_buffer_.empty()) return nullptr;

    // 丢弃过旧帧
    while(frame_buffer_.size() > 1 && frame_buffer_.front().pts_us + kDropToleranceUs < requested_pts_us) {
        frame_buffer_.pop_front();
    }

    // 查找最接近的帧返回
    std::shared_ptr<cv::Mat> best_frame = nullptr;
    int64_t min_diff = INT64_MAX;
    for(auto& item : frame_buffer_) {
        int64_t diff = std::abs(item.pts_us - requested_pts_us);
        if(diff < min_diff) {
            min_diff = diff;
            best_frame = item.frame_data;
        }
    }

    return best_frame;
}
