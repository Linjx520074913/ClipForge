#ifndef __CFPLAYER_H__
#define __CFPLAYER_H__

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
}

#define CF_A_PAUSE (1 << 0)  // 音频解码暂停，audio decode thread 检测到此状态会暂停音频解码
#define CF_V_PAUSE (1 << 1)  // 视频解码暂停，video decode thread 检测到此状态会暂停视频解码
#define CF_R_PAUSE (1 << 2)  // 控制视频或音频输出是否暂停
#define CF_F_SEEK  (1 << 3)  // 全局 seek 标志，表示播放器正在执行 seek 操作，解码线程需要调整 pts
#define CF_A_SEEK  (1 << 4)  // 音频 seek 标志，音频解码线程检测到此标志进行音频 seek
#define CF_V_SEEK  (1 << 5)  // 视频 seek 标志，视频解码线程检测到此标志进行视频 seek
#define CF_NONE    (1 << 6)  // 暂无
#define CF_CLOSE   (1 << 7)  // 关闭播放器，所有线程检测到后会退出循环

struct CFFrame {
    int width    = 0;
    int height   = 0;
    int channels = 4;
    int length   = 0;
    std::shared_ptr<uint8_t[]> data;
    double timestamp = 0.0;
};

class CFPlayer
{
public:
    CFPlayer() : v_frames_size_(0) {}

public:
    void open(const char* file_path);
    void close();
    void play();
    void pause();
    void seek();
    std::shared_ptr<CFFrame> get_frame(int64_t timestamp);

private:
    AVHWDeviceType get_hw_device(){
        #if defined(_WIN32)
            // Window 优先 DXVA2 / D3D11VA / CUDA
            if(av_hwdevice_find_type_by_name("dxva2") != AV_HWDEVICE_TYPE_NONE)
                return AV_HWDEVICE_TYPE_DXVA2;
            if(av_hwdevice_find_type_by_name("d3d11va") != AV_HWDEVICE_TYPE_NONE)
                return AV_HWDEVICE_TYPE_D3D11VA;
            if(av_hwdevice_find_type_by_name("cuda") != AV_HWDEVICE_TYPE_NONE)
                return AV_HWDEVICE_TYPE_CUDA;
        #elif define(__APPLE__)

        #else

        #endif
            return AV_HWDEVICE_TYPE_NONE;
    }

    void decode_video_loop();

    static double pts_to_ms(int64_t pts, AVRational tb) {
        return (pts == AV_NOPTS_VALUE) ? 0.0 : static_cast<double>(pts) * av_q2d(tb) * 1000.0;
    }

    double get_buffer_ts_diff(){
        return v_frames_.size() == 0 ? 0 : v_frames_.back().get()->timestamp - v_frames_.front().get()->timestamp;
    }

private:
    // format
    AVFormatContext *fmt_ctx_ = nullptr;

    SwsContext      *sws_ctx_ = nullptr;

    // audio
    AVCodecContext  *a_codec_context_;
    int              a_stream_index_;
    AVRational       a_stream_timebase_;
    AVFrame          a_frame_;

    // video
    AVCodecContext  *v_codec_ctx_;
    int              v_stream_idx_;
    AVRational       v_stream_timebase_;
    AVRational       v_frate_; 

    // 组合状态
    // 例如同时暂停音视频和渲染
    // status = CF_A_PAUSE | CF_V_PAUSE | CF_R_PAUSE
    int              status_;    
    
    int              seek_req_;  // 是否有 seek 请求，用于避免多次同时 seek
    int64_t          seek_pos_;  // 目标流时间戳，用于解码线程快速定位到要解码的帧
    int64_t          seek_dest_; // 目标显示时间戳，表示最终想要跳转到的播放位置
    int64_t          seek_vpts_; // 视频解码线程当前 seek 帧 pts，用于计算视频帧是否已到达目标位置
    int              seek_diff_; // 允许的误差，在 seek 时允许视频/音频帧偏离目标 pts 的范围
    int              seek_sidx_; // 当前 seek 流索引

    void*            pkt_queue_; // 音视频包队列，demux 线程往这放包，音视频解码线程从这里取包

    std::thread      avdemux_thread_;
    std::thread      a_decode_thread_;
    std::thread      v_decode_thread_;

    bool             running_;

    std::deque<std::shared_ptr<CFFrame>> v_frames_;
    std::mutex mutex_;
    std::condition_variable cond_v_;

    int              v_frames_diff_; // 视频缓存首尾最大的时间差，相当于缓存的时间
    std::atomic<int> v_frames_size_;
    
};

#endif