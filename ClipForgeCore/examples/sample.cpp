#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>

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

// 硬件格式回调
enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
    for (const enum AVPixelFormat* p = pix_fmts; *p != -1; ++p)
        if (*p == hw_pix_fmt) return *p;
    return AV_PIX_FMT_NONE;
}

// 初始化 NVDEC
int init_hw_decoder(AVCodecContext* ctx) {
    AVHWDeviceType type = av_hwdevice_find_type_by_name("cuda");
    if (av_hwdevice_ctx_create(&hw_device_ctx, type, nullptr, nullptr, 0) < 0) {
        std::cerr << "Failed to create HW device\n";
        return -1;
    }
    ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    return 0;
}

int main() {
    const char* filename = "D://video//Q360_20250612_141124_000001.MP4";

    avformat_network_init();
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) != 0) {
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
    AVCodecContext* dec_ctx = avcodec_alloc_context3(decoder);
    avcodec_parameters_to_context(dec_ctx, codecpar);

    hw_pix_fmt = AV_PIX_FMT_CUDA;
    dec_ctx->get_format = get_hw_format;
    if (init_hw_decoder(dec_ctx) < 0) return -1;
    avcodec_open2(dec_ctx, decoder, nullptr);

    AVPacket* pkt = av_packet_alloc();
    AVFrame* hw_frame = av_frame_alloc();
    AVFrame* sw_frame = av_frame_alloc();
    AVFrame* rgb_frame = av_frame_alloc();

    // sws 将 CPU NV12 -> RGB24
    struct SwsContext* sws_ctx = sws_getContext(
        dec_ctx->width, dec_ctx->height, AV_PIX_FMT_NV12,
        dec_ctx->width, dec_ctx->height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!sws_ctx) { std::cerr << "Failed to create SwsContext\n"; return -1; }

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, dec_ctx->width, dec_ctx->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(num_bytes);
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, buffer, AV_PIX_FMT_RGB24,
        dec_ctx->width, dec_ctx->height, 1);

    // SDL2 初始化
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("NVDEC + SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        dec_ctx->width, dec_ctx->height,
        SDL_WINDOW_RESIZABLE);
    if (!window) { std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl; return -1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        dec_ctx->width,
        dec_ctx->height);

    SDL_Event e;
    bool running = true;

    while (av_read_frame(fmt_ctx, pkt) >= 0 && running) {
        if (pkt->stream_index == video_index) {
            if (avcodec_send_packet(dec_ctx, pkt) < 0) continue;
            while (avcodec_receive_frame(dec_ctx, hw_frame) == 0) {
                // 硬件帧拷贝到 CPU
                if (av_hwframe_transfer_data(sw_frame, hw_frame, 0) < 0) {
                    std::cerr << "Error transferring frame to CPU\n";
                    continue;
                }

                // CPU NV12 -> RGB24
                sws_scale(sws_ctx, sw_frame->data, sw_frame->linesize, 0, dec_ctx->height,
                    rgb_frame->data, rgb_frame->linesize);

                // 更新 SDL2 纹理显示
                SDL_UpdateTexture(texture, nullptr, rgb_frame->data[0], rgb_frame->linesize[0]);
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);
                SDL_RenderPresent(renderer);

                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) running = false;
                }
            }
        }
        av_packet_unref(pkt);
    }

    // 清理
    sws_freeContext(sws_ctx);
    av_free(buffer);
    av_frame_free(&rgb_frame);
    av_frame_free(&sw_frame);
    av_frame_free(&hw_frame);
    av_packet_free(&pkt);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
