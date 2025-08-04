#include "video_decoder.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

VideoDecoder::VideoDecoder() = default;
VideoDecoder::~VideoDecoder() { Uninitialize(); }

bool VideoDecoder::Initialize(const char* file_name)
{
    if (is_running_) return false;

    if (avformat_open_input(&format_context_, file_name, nullptr, nullptr) < 0) {
        cerr << "Failed to open input file: " << file_name << endl;
        return false;
    }

    if (avformat_find_stream_info(format_context_, nullptr) < 0) {
        cerr << "Failed to find stream info" << endl;
        Cleanup();
        return false;
    }

    video_stream_index_ = av_find_best_stream(format_context_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index_ < 0) {
        cerr << "No video stream found" << endl;
        Cleanup();
        return false;
    }

    AVStream* stream = format_context_->streams[video_stream_index_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        cerr << "Codec not found" << endl;
        Cleanup();
        return false;
    }

    codec_context_ = avcodec_alloc_context3(decoder);
    if (!codec_context_) {
        cerr << "Could not allocate codec context" << endl;
        Cleanup();
        return false;
    }

    if (avcodec_parameters_to_context(codec_context_, stream->codecpar) < 0) {
        cerr << "Failed to copy codec parameters" << endl;
        Cleanup();
        return false;
    }

    if (avcodec_open2(codec_context_, decoder, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        Cleanup();
        return false;
    }

    sws_context_ = nullptr;
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;

    is_running_ = true;
    decode_thread_ = std::thread(&VideoDecoder::DecodeThreadLoop, this);

    return true;
}

void VideoDecoder::Uninitialize()
{
    if (!is_running_) return;

    is_running_ = false;
    cv_frame_.notify_all();

    if (decode_thread_.joinable()) decode_thread_.join();
    Cleanup();
}

void VideoDecoder::Cleanup()
{
    if (codec_context_) {
        avcodec_free_context(&codec_context_);
        codec_context_ = nullptr;
    }
    if (format_context_) {
        avformat_close_input(&format_context_);
        format_context_ = nullptr;
    }
    if (sws_context_) {
        sws_freeContext(sws_context_);
        sws_context_ = nullptr;
    }
    frame_buffer_.clear();
    current_src_pix_fmt_ = AV_PIX_FMT_NONE;
    request_seek_ = false;
    last_pts_us_ = 0;
}

void VideoDecoder::DecodeThreadLoop()
{
    while (is_running_) {
        DecodeNextFrames(request_seek_);
        request_seek_ = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // 避免CPU满载
    }
}

void VideoDecoder::DecodeNextFrames(bool do_seek)
{
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    if (do_seek) {
        int64_t seek_target = av_rescale_q(seek_target_us_, AVRational{1, 1000000},
                                           format_context_->streams[video_stream_index_]->time_base);
        av_seek_frame(format_context_, video_stream_index_, seek_target, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(codec_context_);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            frame_buffer_.clear();
        }
    }

    while (is_running_) {
        if (av_read_frame(format_context_, pkt) < 0) break;

        if (pkt->stream_index != video_stream_index_) {
            av_packet_unref(pkt);
            continue;
        }

        if (avcodec_send_packet(codec_context_, pkt) == 0) {
            while (avcodec_receive_frame(codec_context_, frame) == 0) {
                int64_t pts_us = av_rescale_q(frame->pts,
                                              format_context_->streams[video_stream_index_]->time_base,
                                              AVRational{1, 1000000});
                last_pts_us_ = pts_us;

                AVPixelFormat src_fmt = (AVPixelFormat)frame->format;
                if (!sws_context_ || src_fmt != current_src_pix_fmt_) {
                    if (sws_context_) sws_freeContext(sws_context_);
                    sws_context_ = sws_getContext(frame->width, frame->height, src_fmt,
                                                  frame->width, frame->height, AV_PIX_FMT_BGR24,
                                                  SWS_BILINEAR, nullptr, nullptr, nullptr);
                    current_src_pix_fmt_ = src_fmt;
                }

                auto mat = std::make_shared<cv::Mat>(frame->height, frame->width, CV_8UC3);
                uint8_t* dst[4] = {mat->data, nullptr, nullptr, nullptr};
                int dst_linesize[4] = { (int)mat->step, 0, 0, 0 };
                sws_scale(sws_context_, frame->data, frame->linesize, 0, frame->height, dst, dst_linesize);

                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    frame_buffer_.emplace_back(VideoFrame{pts_us, mat});
                    if (frame_buffer_.size() > max_cache_frames_) frame_buffer_.pop_front();
                }
                cv_frame_.notify_all();
            }
        }

        av_packet_unref(pkt);

        // 缓冲足够时暂停解码，避免过度解码导致卡顿
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (frame_buffer_.size() > max_cache_frames_ - 10) break;
        }
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}

std::shared_ptr<cv::Mat> VideoDecoder::GetFrame(int time_ms, int timeout_ms)
{
    int64_t requested_pts_us = (int64_t)time_ms * 1000;

    // 判断是否需要seek
    if (std::abs(requested_pts_us - last_pts_us_) > kSeekThresholdUs) {
        request_seek_ = true;
        seek_target_us_ = requested_pts_us;
    }

    // 等待帧到来
    std::unique_lock<std::mutex> lock(mutex_);
    cv_frame_.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this, requested_pts_us]() {
        if (!is_running_ || frame_buffer_.empty()) return false;
        for (auto& f : frame_buffer_) {
            if (std::abs(f.pts_us - requested_pts_us) <= kDropToleranceUs) return true;
        }
        return false;
    });

    if (frame_buffer_.empty()) return nullptr;

    // 选最接近的帧
    std::shared_ptr<cv::Mat> best = nullptr;
    int64_t min_diff = INT64_MAX;
    for (auto& f : frame_buffer_) {
        int64_t diff = std::abs(f.pts_us - requested_pts_us);
        if (diff < min_diff) {
            min_diff = diff;
            best = f.frame_data;
        }
    }
    return best;
}
