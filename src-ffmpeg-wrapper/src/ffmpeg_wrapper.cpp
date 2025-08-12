#define FFMPEG_WRAPPER_DLL

#include "av_decoder.h"
#include "ffmpeg_wrapper.h"

#include <opencv2/opencv.hpp>


using namespace std;



const char* get_version()
{
    return AVDecoder::get_version();
}

const char* get_av_meta_data(const char* file_path)
{
    return AVDecoder::get_av_meta_data(file_path);
}

void free_av_meta_data(const char* ptr)
{
    return AVDecoder::free_av_meta_data(ptr);
}

AVFormatContext *fmt_ctx = nullptr;
AVCodecContext *video_dec_ctx = nullptr;
int videoStreamIdx = -1;

static AVPacket *pkt = nullptr;
static AVFrame *frame = nullptr;
static AVFrame *rgb_frame = nullptr;
static struct SwsContext *sws_ctx = nullptr;
static uint8_t *rgb_buffer = nullptr;

// 释放资源函数
void cleanup() {
    if (rgb_buffer) {
        av_free(rgb_buffer);
        rgb_buffer = nullptr;
    }
    if (rgb_frame) {
        av_frame_free(&rgb_frame);
        rgb_frame = nullptr;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (pkt) {
        av_packet_free(&pkt);
        pkt = nullptr;
    }
    if (video_dec_ctx) {
        avcodec_free_context(&video_dec_ctx);
        video_dec_ctx = nullptr;
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
}

void open_video(const char* filePath)
{
    int ret = 0;

    // 初始化对象
    pkt = av_packet_alloc();
    frame = av_frame_alloc();
    rgb_frame = av_frame_alloc();

    if (!pkt || !frame || !rgb_frame) {
        cerr << "Failed to allocate AVPacket or AVFrame" << endl;
        cleanup();
        return;
    }

    if ((ret = avformat_open_input(&fmt_ctx, filePath, NULL, NULL)) < 0) {
        cerr << "avformat_open_input error: " << filePath << endl;
        cleanup();
        return;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        cerr << "avformat_find_stream_info error: " << filePath << endl;
        cleanup();
        return;
    }

    cout << "Total ms = " << (fmt_ctx->duration / (AV_TIME_BASE / 1000)) << endl;

    videoStreamIdx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoStreamIdx < 0) {
        cerr << "Could not find video stream" << endl;
        cleanup();
        return;
    }

    AVStream *videoStream = fmt_ctx->streams[videoStreamIdx];
    const AVCodec *decoder = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if (!decoder) {
        cerr << "Codec not found" << endl;
        cleanup();
        return;
    }

    video_dec_ctx = avcodec_alloc_context3(decoder);
    if (!video_dec_ctx) {
        cerr << "Could not allocate codec context" << endl;
        cleanup();
        return;
    }

    if ((ret = avcodec_parameters_to_context(video_dec_ctx, videoStream->codecpar)) < 0) {
        cerr << "avcodec_parameters_to_context failed" << endl;
        cleanup();
        return;
    }

    if ((ret = avcodec_open2(video_dec_ctx, decoder, NULL)) < 0) {
        cerr << "Could not open codec" << endl;
        cleanup();
        return;
    }

    sws_ctx = sws_getContext(
        video_dec_ctx->width,
        video_dec_ctx->height,
        video_dec_ctx->pix_fmt,
        video_dec_ctx->width,
        video_dec_ctx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        cerr << "Could not initialize sws context" << endl;
        cleanup();
        return;
    }

    int rgb_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, video_dec_ctx->width, video_dec_ctx->height, 1);
    rgb_buffer = (uint8_t*)av_malloc(rgb_buffer_size);
    if (!rgb_buffer) {
        cerr << "Could not allocate RGB buffer" << endl;
        cleanup();
        return;
    }

    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, rgb_buffer, AV_PIX_FMT_RGB24,
                         video_dec_ctx->width, video_dec_ctx->height, 1);
    rgb_frame->width = video_dec_ctx->width;
    rgb_frame->height = video_dec_ctx->height;
    rgb_frame->format = AV_PIX_FMT_RGB24;

    while (av_read_frame(fmt_ctx, pkt) >= 0) {
        if (pkt->stream_index == videoStreamIdx) {
            if (avcodec_send_packet(video_dec_ctx, pkt) == 0) {
                while (avcodec_receive_frame(video_dec_ctx, frame) == 0) {
                    cout << "Decoded frame: pts=" << frame->pts 
                         << " w=" << frame->width 
                         << " h=" << frame->height << endl;

                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                              rgb_frame->data, rgb_frame->linesize);

                    cv::Mat mat(rgb_frame->height, rgb_frame->width, CV_8UC3, rgb_frame->data[0], rgb_frame->linesize[0]);
                    cv::imshow("Video", mat);
                    if (cv::waitKey(1) == 27) { // ESC键退出
                        cleanup();
                        return;
                    }
                    av_frame_unref(frame);
                }
            }
        }
        av_packet_unref(pkt);
    }

    cout << "Decoding finished!" << endl;
    cleanup();
}

void get_frame(int millisecond)
{

}

void close_video()
{

}