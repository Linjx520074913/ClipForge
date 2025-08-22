#include "CFPlayer.h"
#include <iostream>
#include <algorithm>

#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>

using namespace std;

void CreateMiniDump(EXCEPTION_POINTERS* pep)
{
    // dump 文件名
    SYSTEMTIME st;
    GetLocalTime(&st);
    TCHAR szFileName[MAX_PATH];
    _stprintf_s(szFileName, _T("CFPlayerDump_%04d%02d%02d_%02d%02d%02d.dmp"),
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, nullptr,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if(hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId           = GetCurrentThreadId();
        mdei.ExceptionPointers  = pep;
        mdei.ClientPointers     = FALSE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                          hFile, MiniDumpNormal, &mdei, nullptr, nullptr);

        CloseHandle(hFile);
    }
}

LONG WINAPI MyExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
    CreateMiniDump(pExceptionPointers);
    return EXCEPTION_EXECUTE_HANDLER; // 或 EXCEPTION_CONTINUE_SEARCH
}

void CFPlayer::open(const char* file_path) {
    SetUnhandledExceptionFilter(MyExceptionFilter);
    if (avformat_open_input(&fmt_ctx_, file_path, nullptr, nullptr) < 0) return;
    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) return;

    v_stream_idx_ = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (v_stream_idx_ < 0) return;

    AVStream* stream = fmt_ctx_->streams[v_stream_idx_];
    const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) return;

    v_codec_ctx_ = avcodec_alloc_context3(decoder);
    if (!v_codec_ctx_) return;
    if (avcodec_parameters_to_context(v_codec_ctx_, stream->codecpar) < 0) return;
    if (avcodec_open2(v_codec_ctx_, decoder, nullptr) < 0) return;

    v_frames_diff_ = 1000; // 最大缓存时间(ms)
    running_ = true;

    // 启动解码线程
    v_decode_thread_ = thread(&CFPlayer::decode_video_loop, this);
}

void CFPlayer::close() {
    running_ = false;
    cond_v_.notify_all();
    if (v_decode_thread_.joinable()) v_decode_thread_.join();

    if (v_codec_ctx_) avcodec_free_context(&v_codec_ctx_);
    if (fmt_ctx_) avformat_close_input(&fmt_ctx_);
    if (sws_ctx_) { sws_freeContext(sws_ctx_); sws_ctx_ = nullptr; }

    lock_guard<mutex> lk(mutex_);
    v_frames_.clear();
    v_frames_size_.store(0);
}

std::shared_ptr<CFFrame> CFPlayer::get_frame(int64_t timestamp)
{
    seek_pos_ = timestamp;
    std::unique_lock<std::mutex> lk(mutex_);
    
    bool need_decode = false; 
    if(get_buffer_ts_diff() > v_frames_diff_ 
    && std::llabs(v_frames_.back().get()->timestamp - timestamp) < v_frames_diff_ / 2) {
        // 清除旧的帧
        v_frames_.erase(
            std::remove_if(v_frames_.begin(), v_frames_.end(), 
            [this](const std::shared_ptr<CFFrame>& f) {
                return seek_pos_ - f.get()->timestamp > v_frames_diff_ / 4;
            }), v_frames_.end());
        // cout << " @@@@@@@@@@@@@@@@@@@ need decoding @@@@@@@@@@@@@@@@@@@@@@" << endl;
        need_decode = true;
    }
    if(get_buffer_ts_diff() > v_frames_diff_  && timestamp > v_frames_.back().get()->timestamp) {
        v_frames_.clear();
        need_decode = true;
    }

    // 找最接近的帧
    auto best = v_frames_.empty() ? nullptr :
        *std::min_element(v_frames_.begin(), v_frames_.end(),
            [timestamp](const std::shared_ptr<CFFrame>& a, const std::shared_ptr<CFFrame>& b){
                return abs(a->timestamp - timestamp) < abs(b->timestamp - timestamp);
            });
    if(need_decode) {
        cond_v_.notify_all();
    }
    int64_t diff = std::llabs(v_frames_.back().get()->timestamp - timestamp);
    
    if(best){
        // cout << "############ get_frame " << "   " << timestamp << " " << best.get()->timestamp << " #################" << endl;
    }
    
    return best;
}

void CFPlayer::decode_video_loop() {
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVStream* stream = fmt_ctx_->streams[v_stream_idx_];

    while (running_) {

        if (av_read_frame(fmt_ctx_, pkt) < 0) {
            running_ = false;
            break;
        }

        if (pkt->stream_index == v_stream_idx_) {
            if (avcodec_send_packet(v_codec_ctx_, pkt) < 0) { av_packet_unref(pkt); continue; }

            while (avcodec_receive_frame(v_codec_ctx_, frame) == 0) {
                auto f = make_shared<CFFrame>();
                f->width = frame->width;
                f->height = frame->height;
                f->channels = 4;
                f->length = f->width * f->height * 4;
                f->data = make_shared<uint8_t[]>(f->length);
                f->timestamp = pts_to_ms(frame->pts, stream->time_base);

                if (!sws_ctx_) {
                    sws_ctx_ = sws_getContext(f->width, f->height, static_cast<AVPixelFormat>(frame->format),
                                              f->width, f->height, AV_PIX_FMT_RGBA,
                                              SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                }

                uint8_t* dst[4] = { f->data.get(), nullptr, nullptr, nullptr };
                int linesize[4] = { f->width*4, 0, 0, 0 };
                sws_scale(sws_ctx_, frame->data, frame->linesize, 0, f->height, dst, linesize);

                {
                    std::unique_lock<std::mutex> lk(mutex_);
                    

                    cond_v_.wait(lk, [this] {
                        return v_frames_.size() < 2  || get_buffer_ts_diff() < v_frames_diff_;
                    });
                    // cout << "push >>> " << f->timestamp << endl;
                    v_frames_.push_back(std::move(f));
                }
            }
        }

        av_packet_unref(pkt);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
}
