#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "application.h"

#include <thread>

typedef websocketpp::server<websocketpp::config::asio> server;

Application::Application()
{
    hwnd_ = nullptr;
    ws_callback_ = nullptr;

    init_websocket();
    init_window();
}

void Application::set_ws_callback(WSMessageCallback callback)
{
    ws_callback_ = callback;
}

int Application::init_websocket()
{
    std::thread t([this](){
        server echo_server;

        echo_server.init_asio();

        echo_server.set_open_handler([](websocketpp::connection_hdl hdl) {
            std::cout << "Client connected\n";
        });

        echo_server.set_message_handler([&echo_server, this](websocketpp::connection_hdl hdl, server::message_ptr msg) {
            std::string payload = msg->get_payload();

            try {
                json j = json::parse(payload);
                if(!ws_callback_) {
                    ws_callback_(j);
                }

                const std::string event = j["event"];
                if(event == "set_size") {
                    int width  = j["data"]["w"];
                    int height = j["data"]["h"];

                    std::cout << "W = " << width << " H = " << height << std::endl;

                    // 调整窗口大小
                    if(hwnd_) {
                        SetWindowPos(hwnd_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
                    }
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

    return 0;
}

int Application::init_window()
{

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW; // 窗口拉伸时触发重绘
    wc.lpfnWndProc = wndproc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "application";

    RegisterClassEx(&wc);

    /**
     * WS_OVERLAPPEDWINDOW 是以下的组合
     * - WS_OVERLAPPED：基础窗口（带标题栏）
     * - WS_CAPTION：标题栏（含文字
     * - WS_SYSMENU：系统菜单（左上角的小图标 + 右键菜单
     * - WS_THICKFRAME：可调整大小的边框
     * - WS_MINIMIZEBOX：最小化按钮
     * - WS_MAXIMIZEBOX：最大化按钮
     */
    hwnd_ = CreateWindow(
        wc.lpszClassName,
        wc.lpszClassName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        300,
        300,
        nullptr, // 父窗口句柄
        nullptr, // 菜单
        wc.hInstance,
        nullptr  
    );

    ShowWindow(hwnd_, SW_SHOW);

    return 0;
}

int Application::run()
{
    MSG msg = {};
    while(msg.message != WM_QUIT) {
        while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }// 每帧渲染
    }
    return 0;
}

LRESULT CALLBACK Application::wndproc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message){
        case WM_CREATE:
            {
                LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lparam);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));   
            }
            break;
        case WM_PAINT:
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE:
            break;
        default:
            break;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}

