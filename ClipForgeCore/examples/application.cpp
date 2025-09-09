#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "application.h"
#include "config.h"

#include <thread>

typedef websocketpp::server<websocketpp::config::asio> server;

Application::Application()
{
    hwnd_ = nullptr;
    renderer_ = nullptr;

    SetProcessDPIAware();

    init_config();
    init_websocket();
    init_window();

    POINT pt = { x_, y_ };
    ClientToScreen(GetParent(hwnd_), &pt);
    SetWindowPos(hwnd_, nullptr, pt.x, pt.y, w_, h_, SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
    ShowWindow(hwnd_, SW_SHOW);

    subscribe("set_size", [this](json& data){
        int width  = data["data"]["w"];
        int height = data["data"]["h"];
        std::cout << "W = " << width << " H = " << height << std::endl;
        if(hwnd_) {
            SetWindowPos(hwnd_, nullptr, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_ASYNCWINDOWPOS);
        }

        if(renderer_) {
            renderer_->resize();
        }
    });
    subscribe("set_pos", [this](json& data){
        int x = data["data"]["x"];
        int y = data["data"]["y"];
        POINT pt = { x, y };
        ClientToScreen(GetParent(hwnd_), &pt);

        RECT rc;
        GetWindowRect(hwnd_, &rc);
        SetWindowPos(hwnd_, nullptr, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER);
    });

}

void Application::subscribe(std::string event, WSMessageCallback callback)
{
    callback_[event] = callback;
}

int Application::init_websocket()
{
    std::thread t([this](){
        server echo_server;

        echo_server.init_asio();

        echo_server.set_open_handler([&echo_server](websocketpp::connection_hdl hdl) {
            std::cout << "Client connected\n";
            echo_server.send(hdl, "connect", websocketpp::frame::opcode::text);
        });

        echo_server.set_message_handler([&echo_server, this](websocketpp::connection_hdl hdl, server::message_ptr msg) {
            std::string payload = msg->get_payload();

            try {
                json j = json::parse(payload);

                const std::string event = j["event"];
                WSMessageCallback cb = callback_[event];
                if(cb) {
                    cb(j);
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

    HWND parent = FindWindow(NULL, "Clipforge");
    if(!parent) { 
        MessageBoxA(0, "No parent window called clipforge found", "FindWindow", MB_ICONERROR | MB_OK);
    }
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
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        300,
        300,
        parent, // 父窗口句柄
        nullptr, // 菜单
        wc.hInstance,
        nullptr  
    );

    return 0;
}

int Application::init_config()
{
    Config cfg;
    if (!cfg.load()) {
        std::cerr << "load failed" << std::endl;
    }

    const auto& data = cfg.get();
    std::cout << "Window Size: " << data.render_wnd_size.w << "x" << data.render_wnd_size.h << "\n";
    std::cout << "Window Pos: (" << data.render_wnd_pos.x << "," << data.render_wnd_pos.y << ")\n";
    std::cout << "Theme: " << data.theme << "\n";

    w_ = data.render_wnd_size.w;
    h_ = data.render_wnd_size.h;
    x_ = data.render_wnd_pos.x;
    y_ = data.render_wnd_pos.y;

    return 0;

}

int Application::run(IRenderer* renderer)
{
    renderer_ = renderer;
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

