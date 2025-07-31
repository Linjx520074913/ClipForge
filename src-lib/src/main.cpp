#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <opencv2/opencv.hpp>

using namespace std;

int main() 
{
    // 使用 av_version_info 获取版本字符串
    cout << "FFmpeg version: " << av_version_info() << endl;

    // 还可以打印具体库的版本号
    unsigned libavcodec_ver = avcodec_version();
    cout << "libavcodec version: "
              << AV_VERSION_MAJOR(libavcodec_ver) << "."
              << AV_VERSION_MINOR(libavcodec_ver) << "."
              << AV_VERSION_MICRO(libavcodec_ver) << endl;

    const char* fileName = "E://test.MP4";

    AVFormatContext* fmt_ctx = nullptr;
    if(avformat_open_input(&fmt_ctx, fileName, nullptr, nullptr) < 0) {
        cerr << "Failed to open input file: " << fileName << endl;
        return -1;
    }

    if(avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        cerr << "Failed to find stream information" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 打印文件信息
    av_dump_format(fmt_ctx, 0, fileName, 0);

    // 找到视频流索引
    int video_stream_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if(video_stream_index < 0) {
        cerr << "Failed to find video stream" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 取出视频流参数，找解码器
    AVStream* video_stream = fmt_ctx->streams[video_stream_index];
    AVCodecParameters* codec_par = video_stream->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
    if(!codec) {
        cerr << "Unsupported codec" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 创建解码器上下文
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx) {
        cerr << "Failed to allocate codec context" << endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    if(avcodec_parameters_to_context(codec_ctx, codec_par) < 0) {
        cerr << "Failed to copy codec params to context" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 打开解码器
    if(avcodec_open2(codec_ctx, codec, nullptr) < 0) {
        cerr << "Failed to open codec" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 分配 AVPacket 和 AVFrame
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* frame_bgr = av_frame_alloc();

    if(!pkt || !frame || !frame_bgr) {
        cerr << "Failed to allocat packet or frame" << endl;
        avcodec_free_context(&codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 设置转换上下文，将解码后的像素格式转成 BGR24
    SwsContext* sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    // 为 BGR frame 分配缓冲区
    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(frame_bgr->data, frame_bgr->linesize, buffer, AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);

    // 创建 opencv 窗口

    while(av_read_frame(fmt_ctx, pkt) >= 0) {
        if(pkt->stream_index == video_stream_index) {
            // 发送数据包到解码器
            if(avcodec_send_packet(codec_ctx, pkt) == 0) {
                // 从解码器接收帧
                while(avcodec_receive_frame(codec_ctx, frame) == 0) {
                    // 转换格式到 BGR
                    sws_scale(sws_ctx, frame->data, frame->linesize, 0, codec_ctx->height, 
                              frame_bgr->data, frame_bgr->linesize);

                    cv::Mat img(codec_ctx->height, codec_ctx->width, CV_8UC3, frame_bgr->data[0], frame_bgr->linesize[0]);
                    cv::imshow("Video", img);
                    cout << "1" << endl;
                    cv::waitKey(1);
                }
            }   
        }
        av_packet_unref(pkt); 
    }
    
    avformat_close_input(&fmt_ctx);
    
    return 0;
}