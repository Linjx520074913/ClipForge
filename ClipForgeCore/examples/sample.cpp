#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>
#include <thread>
#include <string>
#include <functional>
#include "Decoder/CFDecoder.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;

using MessageHandler = std::function<void(json& data)>;

void init_websocket(MessageHandler handler)
{
    std::thread t([handler](){
        server echo_server;

        // 初始化 Asio
        echo_server.init_asio();

        // 连接处理
        echo_server.set_open_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "Client connected\n";
            });

        echo_server.set_message_handler([&echo_server, handler](websocketpp::connection_hdl hdl, server::message_ptr msg) {
            std::string payload = msg->get_payload();

            try {
                json j = json::parse(payload);
                if(handler) {
                    handler(j);
                }
            } catch (std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
            // echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
        });

        echo_server.listen(9002);
        echo_server.start_accept();

        echo_server.run();
    });
    t.detach();
}

AVFrame* getFrame(const char* file_path)
{
    AVFormatContext* fmt_ctx = nullptr;
    avformat_open_input(&fmt_ctx, file_path, NULL, NULL);
    avformat_find_stream_info(fmt_ctx, NULL);

    int video_stream_idx;
    AVCodecContext* v_codec_ctx = nullptr;
    for(int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        if(stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
            video_stream_idx = i;
            v_codec_ctx = avcodec_alloc_context3(codec);
            avcodec_parameters_to_context(v_codec_ctx, fmt_ctx->streams[i]->codecpar);
            avcodec_open2(v_codec_ctx, codec, NULL);
        }
    }
    int i = 100;
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while(1) {
        
        int ret = av_read_frame(fmt_ctx, pkt);
        if(ret == 0 && pkt->stream_index == video_stream_idx) {
            if(0 == avcodec_send_packet(v_codec_ctx, pkt)) {
                if(0 == avcodec_receive_frame(v_codec_ctx, frame) && i-- == 0) {
                    av_packet_unref(pkt);
                    avcodec_free_context(&v_codec_ctx);
                    avformat_close_input(&fmt_ctx);
                    return frame;
                }
            }
        }

        av_packet_unref(pkt);
    }
}


int main()
{
    AVFrame* frame = getFrame("D://video//video.mp4");
    // 避免窗口在不同 DPI 上进行缩放从而导致模糊
    SetProcessDPIAware();

    // 1.1 注册窗口类
    HWND parent = FindWindow(NULL, "clipforge"); // tauri 窗口

    WNDCLASS wc = {};
    // wc.lpfnWndProc = WndPro
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "window";
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    };;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if(!RegisterClass(&wc)) {
        std::cerr << "RegisterClass failed, error = " << GetLastError() << std::endl;
        return -1;
    }


    // 1.2 创建窗口
    HWND hwnd = CreateWindow(
        "window",
        "MyRenderer",
        WS_POPUP  | WS_VISIBLE,
        0, 0, 1280, 720,
        parent, NULL, wc.hInstance, NULL
    );
    // SetParent(hwnd, parent); 

    if(!hwnd) {
        std::cerr << "CreateWindow failed, error = " << GetLastError() << std::endl;
        return -1;
    }

    init_websocket([=](json& data){
        const string event = data["event"];
        if(event == "set_size") {
            int w = data["data"]["w"];
            int h = data["data"]["h"];
            

            SetWindowPos(hwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);  
        }else if(event == "set_pos") {
            int x = data["data"]["x"];
            int y = data["data"]["y"];
            RECT rc;
            GetWindowRect(hwnd, &rc);
            SetWindowPos(hwnd, nullptr, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);

        }
    });

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {};
    while(GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}