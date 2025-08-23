#include <iostream>
#include <SDL3/SDL.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

// 全局硬件设备
AVBufferRef* hw_device_ctx = nullptr;
enum AVPixelFormat hw_pix_fmt;

// 选择硬件格式回调
enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
    for (const enum AVPixelFormat* p = pix_fmts; *p != -1; ++p)
        if (*p == hw_pix_fmt) return *p;
    return AV_PIX_FMT_NONE;
}

// 初始化 NVDEC
int init_hw_decoder(AVCodecContext* ctx) {
    AVHWDeviceType type = av_hwdevice_find_type_by_name("cuda");
    if (av_hwdevice_ctx_create(&hw_device_ctx, type, nullptr, nullptr, 0) < 0) {
        std::cerr << "Failed to create HW device" << std::endl;
        return -1;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return 0;
}

int main(int argc, char* argv[]) {

    avformat_network_init();
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, "D://video//Q360_20250612_141124_000001.MP4", nullptr, nullptr) != 0) {
        std::cerr << "Cannot open input file.\n";
        return -1;
    }
    avformat_find_stream_info(fmt_ctx, nullptr);

    int video_index = -1;
    for (unsigned i = 0; i < fmt_ctx->nb_streams; ++i)
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_index = i;
            break;
        }
    if (video_index < 0) { std::cerr << "No video stream.\n"; return -1; }

    AVCodecParameters* codecpar = fmt_ctx->streams[video_index]->codecpar;
    const AVCodec* decoder = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext* decoder_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(decoder_ctx, codecpar);

    hw_pix_fmt = AV_PIX_FMT_CUDA;
    decoder_ctx->get_format = get_hw_format;
    if (init_hw_decoder(decoder_ctx) < 0) return -1;
    avcodec_open2(decoder_ctx, decoder, nullptr);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* rgb_frame = av_frame_alloc();

    // sws 将 NV12 -> RGB24
    struct SwsContext* sws_ctx = sws_getContext(
        decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_NV12,
        decoder_ctx->width, decoder_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, decoder_ctx->width, decoder_ctx->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24, decoder_ctx->width, decoder_ctx->height, 1);

    // SDL3 初始化
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "NVDEC SDL3",
        decoder_ctx->width,
        decoder_ctx->height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) { std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl; return -1; }

    SDL_Surface* screen_surface = SDL_GetWindowSurface(window);

    bool running = true;
    SDL_Event e;

    while (av_read_frame(fmt_ctx, pkt) >= 0 && running) {
        if (pkt->stream_index == video_index) {
            if (avcodec_send_packet(decoder_ctx, pkt) < 0) continue;
            while (avcodec_receive_frame(decoder_ctx, frame) == 0) {
                // NV12 -> RGB24
                sws_scale(sws_ctx, frame->data, frame->linesize, 0, decoder_ctx->height,
                          rgb_frame->data, rgb_frame->linesize);

                // 拷贝到 SDL_Surface
                memcpy(screen_surface->pixels, rgb_frame->data[0], rgb_frame->linesize[0] * decoder_ctx->height);
                SDL_UpdateWindowSurface(window);

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_EVENT_QUIT) running = false;
                }
            }
        }
        av_packet_unref(pkt);
    }

    // 清理资源
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&rgb_frame);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&decoder_ctx);
    avformat_close_input(&fmt_ctx);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
