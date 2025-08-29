#include "decoder.h"
#include <windows.h>
#include <iostream>
#include <thread>

void Decoder::open_video(const char* file_path, AVFrameCallback callback)
{
    std::thread t([=](){
        int res = avformat_open_input(&fmt_ctx_, file_path, nullptr, nullptr);
        avformat_find_stream_info(fmt_ctx_, NULL);

        int v_stream_idx = 0;
        int a_stream_idx = 0;
        AVCodecContext*  v_codec_ctx;
        AVCodecContext*  a_codec_ctx;

        double avg_frame_rate;
        
        for(int i = 0; i < fmt_ctx_->nb_streams; i++) {
            auto stream = fmt_ctx_->streams[i];
            AVCodec* codec = (AVCodec*)avcodec_find_decoder(stream->codecpar->codec_id);
            switch(codec->type) {
                case AVMEDIA_TYPE_VIDEO:
                    {
                        v_stream_idx = i;
                        avg_frame_rate = (double)stream->avg_frame_rate.den / stream->avg_frame_rate.num;
                        v_codec_ctx = avcodec_alloc_context3(codec);
                        avcodec_parameters_to_context(v_codec_ctx, stream->codecpar);
                        avcodec_open2(v_codec_ctx, codec, NULL);

                        codec_map_[i] = v_codec_ctx;

                        AVBufferRef* hw_device_ctx = nullptr;
                        av_hwdevice_ctx_create(&hw_device_ctx, AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA, NULL, NULL, NULL);
                        if(hw_device_ctx) {
                            v_codec_ctx->hw_device_ctx = hw_device_ctx;
                        }else {
                            MessageBox(NULL, "初始化硬件解码器失败", "错误", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    break;
                default:
                    break;
            }
        }

        int width = v_codec_ctx->width;
        int height= v_codec_ctx->height;

        
        while(true) {
            AVPacket* pkt = av_packet_alloc();
            int ret = av_read_frame(fmt_ctx_, pkt);
            if(0 == ret) {
                auto codec_ctx = codec_map_[pkt->stream_index];
                if(pkt->stream_index == AVMEDIA_TYPE_VIDEO && 0 == avcodec_send_packet(codec_ctx, pkt)) {
                    AVFrame* frame= av_frame_alloc();
                    if(0 == avcodec_receive_frame(codec_ctx, frame)) {
                        av_packet_unref(pkt);
                        // 回调
                        if(callback) {
                            callback(frame);
                        }
                        av_frame_free(&frame);
                    }else {
                        av_frame_unref(frame);
                    }
                }
            }else {
                std::cerr << "##" << std::endl;
                break;
            }
        }

        avcodec_free_context(&v_codec_ctx);
        avformat_close_input(&fmt_ctx_);
    });
    t.detach();
}