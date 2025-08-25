#include <windows.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext_dxva2.h>
}

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

RECT gClientRect = {}; // 窗口客户区
IDirect3DSwapChain9* gSwapChain = nullptr;
IDirect3DDevice9* gD3DDevice = nullptr;

// 窗口消息处理
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &gClientRect);
        // 如果 SwapChain 已存在，重新创建以匹配新尺寸
        if (gSwapChain && gD3DDevice) {
            D3DPRESENT_PARAMETERS pp = {};
            pp.Windowed = TRUE;
            pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            pp.BackBufferFormat = D3DFMT_X8R8G8B8;
            pp.BackBufferWidth = gClientRect.right - gClientRect.left;
            pp.BackBufferHeight = gClientRect.bottom - gClientRect.top;
            pp.hDeviceWindow = hWnd;

            gSwapChain->Release();
            gSwapChain = nullptr;
            gD3DDevice->CreateAdditionalSwapChain(&pp, &gSwapChain);
        }
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// DXVA2 get_format 回调
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    for (const enum AVPixelFormat *p = pix_fmts; *p != -1; p++) {
        if (*p == AV_PIX_FMT_DXVA2_VLD)
            return *p;
    }
    std::cerr << "No DXVA2 format found\n";
    return AV_PIX_FMT_NONE;
}

int main() {
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

    AVBufferRef* hw_device_ctx = nullptr;
    if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, nullptr, nullptr, 0) < 0) {
        std::cerr << "Failed to create DXVA2 device\n"; return -1;
    }
    codec_ctx->hw_device_ctx = hw_device_ctx;
    codec_ctx->get_format = get_hw_format;

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0) return -1;

    AVPacket pkt;
    AVFrame* frame = av_frame_alloc();

    // ------------------- 创建窗口 -------------------
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "DXVA2Window";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName, 
        "DXVA2 GPU Display (Resizable)",
        WS_OVERLAPPEDWINDOW,  // 可调整大小
        CW_USEDEFAULT, CW_USEDEFAULT,
        1280, 720,
        nullptr, nullptr, wc.hInstance, nullptr
    );
    ShowWindow(hwnd, SW_SHOW);

    // ------------------- FFmpeg D3D9 Device -------------------
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

                    IDirect3DSurface9* dxvaSurface = (IDirect3DSurface9*)frame->data[3];
                    if (!dxvaSurface) continue;

                    // ---------------- 拿 FFmpeg 内部 Device（只做一次） ----------------
                    if (!dev_ready) {
                        dxvaSurface->GetDevice(&gD3DDevice);
                        dev_ready = true;

                        // 创建自定义 SwapChain
                        D3DPRESENT_PARAMETERS pp = {};
                        pp.Windowed = TRUE;
                        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                        pp.BackBufferFormat = D3DFMT_X8R8G8B8;
                        pp.BackBufferWidth = gClientRect.right - gClientRect.left;
                        pp.BackBufferHeight = gClientRect.bottom - gClientRect.top;
                        pp.hDeviceWindow = hwnd;

                        gD3DDevice->CreateAdditionalSwapChain(&pp, &gSwapChain);
                    }

                    // ---------------- 计算目标矩形，保持宽高比 ----------------
                    int winW = gClientRect.right - gClientRect.left;
                    int winH = gClientRect.bottom - gClientRect.top;
                    int vidW = codec_ctx->width;
                    int vidH = codec_ctx->height;

                    RECT destRect;
                    float winRatio = (float)winW / winH;
                    float vidRatio = (float)vidW / vidH;

                    if (vidRatio > winRatio) {
                        int h = (int)(winW / vidRatio);
                        int y = (winH - h) / 2;
                        destRect = {0, y, winW, y + h};
                    } else {
                        int w = (int)(winH * vidRatio);
                        int x = (winW - w) / 2;
                        destRect = {x, 0, x + w, winH};
                    }

                    // ---------------- 渲染到 SwapChain ----------------
                    IDirect3DSurface9* backSurface = nullptr;
                    gSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backSurface);

                    HRESULT hr = gD3DDevice->StretchRect(dxvaSurface, nullptr, backSurface, &destRect, D3DTEXF_LINEAR);
                    if (hr == D3DERR_DEVICELOST) {
                        gD3DDevice->TestCooperativeLevel(); // 等待设备恢复
                        backSurface->Release();
                        continue;
                    }

                    gSwapChain->Present(nullptr, nullptr, nullptr, nullptr, 0);
                    backSurface->Release();
                }
            }
            av_packet_unref(&pkt);
        }
    }

    // ------------------- 清理 -------------------
    if (gSwapChain) gSwapChain->Release();
    if (gD3DDevice) gD3DDevice->Release();
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&fmt_ctx);

    return 0;
}
