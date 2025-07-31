#define DECODER_DLL_EXPORTS  // 告诉编译器我们正在编译 DLL
#include "decoder.h"

#include <thread>
#include <format>

using namespace std;

FFMpegVersion get_ffmpeg_version()
{
    unsigned version = avcodec_version();

    FFMpegVersion v;
    v.major = AV_VERSION_MAJOR(version);
    v.minor = AV_VERSION_MINOR(version);
    v.micro = AV_VERSION_MICRO(version);
    return v;
}

void show_frames()
{
    std::thread t([](){
        cout << "FFmpeg version: " << av_version_info() << endl;

        const char* fileName = "E://test.MP4";

        AVFormatContext* fmt_ctx = nullptr;
        if (avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr) < 0) {
            cerr << "[❌] Cannot open file: " << fileName << endl;
            return -1;
        }

        std::format("avformat_open_input : {} success", fileName);

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
    });
    t.detach();
}