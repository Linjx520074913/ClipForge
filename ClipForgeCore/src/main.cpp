#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

using namespace std;

/**
 * [MP4 文件]  
    │
    ▼  avformat_open_input()
    [容器解析: AVFormatContext]
        │
        ▼  avformat_find_stream_info()
    [读取流信息] 
        │
        ▼  av_find_best_stream()
    [找到视频流索引] 
        │
        ▼  avcodec_find_decoder()
    [选择解码器] 
        │
        ▼  avcodec_alloc_context3() + avcodec_parameters_to_context()
    [创建解码上下文 AVCodecContext]
        │
        ▼  avcodec_open2()
    [打开解码器]
        │
        ▼  while(av_read_frame()) → AVPacket
    [循环读取压缩帧]
        │
        ▼  avcodec_send_packet()
    [送入解码器]
        │
        ▼  avcodec_receive_frame()
    [输出解码帧 AVFrame (YUV420p/10bit)]
        │
        ▼  sws_getContext() + sws_scale()
    [像素格式转换: YUV → BGR24]
        │
        ▼  av_image_fill_arrays()
    [准备 OpenCV 可用缓冲区]
        │
        ▼  cv::Mat(img) + cv::imshow()
    [OpenCV 显示帧]
 * 
 */
int main() {
    cout << "FFmpeg version: " << av_version_info() << endl;

    const char* fileName = "E://test.MP4";

    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr) < 0) {
        cerr << "[❌] Cannot open file: " << fileName << endl;
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        cerr << "[❌] Cannot find stream info" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index < 0) {
        cerr << "[❌] No video stream found" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVStream* video_stream = fmt_ctx->streams[video_stream_index];
    const AVCodec* codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (!codec) {
        cerr << "[❌] Unsupported codec" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, video_stream->codecpar);
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cerr << "[❌] Failed to open codec" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 分配资源
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* frame_bgr = av_frame_alloc();

    SwsContext* sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    int buf_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t* buf = (uint8_t*)av_malloc(buf_size);
    av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, buf, AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);

    cv::namedWindow("Video", cv::WINDOW_AUTOSIZE);

    // 读取+解码循环
    auto decode_and_show = [&](AVPacket* p) {
        if (avcodec_send_packet(codec_ctx, p) < 0) return;
        while (avcodec_receive_frame(codec_ctx, frame) == 0) {
            sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, frame_bgr->data, frame_bgr->linesize);
            cv::Mat img(codec_ctx->height, codec_ctx->width, CV_8UC3, frame_bgr->data[0], frame_bgr->linesize[0]);
            cv::imshow("Video", img);
            if (cv::waitKey(1) == 'q') exit(0);
        }
    };

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == video_stream_index) decode_and_show(pkt);
        av_packet_unref(pkt);
    }

    // flush 剩余帧
    decode_and_show(nullptr);

    // ✅ 释放资源
    av_free(buf);
    sws_freeContext(sws_ctx);
    av_frame_free(&frame_bgr);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}
