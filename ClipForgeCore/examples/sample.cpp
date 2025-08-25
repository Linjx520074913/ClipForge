#include <windows.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// DXVA2 get_format 回调
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    for (const enum AVPixelFormat *p = pix_fmts; *p != -1; p++) {
        if (*p == AV_PIX_FMT_DXVA2_VLD)
            return *p;
    }
    std::cerr << "No DXVA2 format found" << std::endl;
    return AV_PIX_FMT_NONE;
}

int main(int argc, char* argv[]) {
    const char* filename = "D://video//Q360_19700103_032841_000001_Output(11).mp4";

    // ------------------- FFmpeg 初始化 -------------------
    avformat_network_init();
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) < 0) return -1;
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) return -1;

    int video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream < 0) return -1;

    AVCodecParameters* codecpar = fmt_ctx->streams[video_stream]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);

    // DXVA2 硬解
    AVBufferRef* hw_device_ctx = nullptr;
    if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, nullptr, nullptr, 0) >= 0) {
        codec_ctx->hw_device_ctx = hw_device_ctx;
        codec_ctx->get_format = get_hw_format;
        std::cout << "DXVA2 hardware decoding enabled\n";
    } else {
        std::cerr << "Failed to create DXVA2 device\n";
        return -1;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) return -1;

    AVPacket pkt;
    AVFrame* frame = av_frame_alloc();

    // ------------------- 创建窗口 -------------------
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "D3DWindowClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, "DXVA2 Direct Display",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             1280, 720, nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    // ------------------- 用 FFmpeg 的 D3DDevice -------------------
    // 取第一帧 DXVA2 surface 来获取 FFmpeg D3DDevice
    IDirect3DDevice9* ffmpegDev = nullptr;
    bool dev_ready = false;

    MSG msg;
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (av_read_frame(fmt_ctx, &pkt) >= 0) {
            if (pkt.stream_index == video_stream) {
                avcodec_send_packet(codec_ctx, &pkt);
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    if (frame->format != AV_PIX_FMT_DXVA2_VLD) continue;

                    IDirect3DSurface9* pSurface = (IDirect3DSurface9*)frame->data[3];
                    if (!pSurface) continue;

                    // 拿 FFmpeg 内部 D3DDevice（只做一次）
                    if (!dev_ready) {
                        pSurface->GetDevice(&ffmpegDev);
                        dev_ready = true;
                    }

                    // 创建或获取窗口 BackBuffer
                    D3DPRESENT_PARAMETERS d3dpp = {};
                    d3dpp.Windowed = TRUE;
                    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                    d3dpp.hDeviceWindow = hwnd;

                    // 直接 StretchRect 到窗口 BackBuffer
                    IDirect3DSurface9* backbuffer = nullptr;
                    ffmpegDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
                    ffmpegDev->StretchRect(pSurface, nullptr, backbuffer, nullptr, D3DTEXF_LINEAR);
                    backbuffer->Release();

                    ffmpegDev->Present(nullptr, nullptr, hwnd, nullptr);
                }
            }
            av_packet_unref(&pkt);
        }
    }

    // ------------------- 清理 -------------------
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    if (ffmpegDev) ffmpegDev->Release();

    return 0;
}
