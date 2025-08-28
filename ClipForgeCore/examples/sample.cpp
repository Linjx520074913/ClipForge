// #include <websocketpp/config/asio_no_tls.hpp>
// #include <websocketpp/server.hpp>

// #include <windows.h>
// #include <thread>
// #include <chrono>
// #include <iostream>
// #include <cstdlib> // rand

// #include <d3d11.h>
// #pragma comment(lib, "d3d11.lib")

// typedef websocketpp::server<websocketpp::config::asio> server;

// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

// using namespace std;

// // ------------------- D3D11 全局 -------------------
// IDXGISwapChain* gSwapChain = nullptr;
// ID3D11Device* gDevice = nullptr;
// ID3D11DeviceContext* gContext = nullptr;
// ID3D11RenderTargetView* gRTV = nullptr;

// HWND gHwnd = nullptr;
// int gWidth = 1280;
// int gHeight = 720;

// // ------------------- D3D11 初始化 -------------------
// bool InitD3D(HWND hwnd, int width, int height) {
//     DXGI_SWAP_CHAIN_DESC scd = {};
//     scd.BufferCount = 1;
//     scd.BufferDesc.Width = width;
//     scd.BufferDesc.Height = height;
//     scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//     scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//     scd.OutputWindow = hwnd;
//     scd.SampleDesc.Count = 1;
//     scd.Windowed = TRUE;
//     scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

//     HRESULT hr = D3D11CreateDeviceAndSwapChain(
//         nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
//         0, nullptr, 0,
//         D3D11_SDK_VERSION, &scd,
//         &gSwapChain, &gDevice, nullptr, &gContext
//     );
//     if (FAILED(hr)) return false;

//     ID3D11Texture2D* backBuffer = nullptr;
//     gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
//     gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);
//     backBuffer->Release();

//     gContext->OMSetRenderTargets(1, &gRTV, nullptr);

//     D3D11_VIEWPORT vp = {};
//     vp.TopLeftX = 0;
//     vp.TopLeftY = 0;
//     vp.Width = (float)width;
//     vp.Height = (float)height;
//     gContext->RSSetViewports(1, &vp);

//     return true;
// }

// // ------------------- 渲染循环 -------------------
// void RenderLoop() {
//     while (true) {
//         float r = rand() / (float)RAND_MAX;
//         float g = rand() / (float)RAND_MAX;
//         float b = rand() / (float)RAND_MAX;
//         float color[4] = { r, g, b, 1.0f };

//         gContext->ClearRenderTargetView(gRTV, color);
//         gSwapChain->Present(1, 0);

//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }

// // ------------------- WebSocket 消息处理 -------------------
// using MessageHandler = std::function<void(json& data)>;

// void init_websocket(MessageHandler handler)
// {
//     std::thread t([handler](){
//         server echo_server;

//         // 初始化 Asio
//         echo_server.init_asio();

//         // 连接处理
//         echo_server.set_open_handler([](websocketpp::connection_hdl hdl) {
//             std::cout << "Client connected\n";
//             });

//         echo_server.set_message_handler([&echo_server, handler](websocketpp::connection_hdl hdl, server::message_ptr msg) {
//             std::string payload = msg->get_payload();

//             try {
//                 json j = json::parse(payload);
//                 if(handler) {
//                     handler(j);
//                 }
//             } catch (std::exception& e) {
//                 std::cerr << "JSON parse error: " << e.what() << std::endl;
//             }
//             // echo_server.send(hdl, msg->get_payload(), msg->get_opcode());
//         });

//         echo_server.listen(9002);
//         echo_server.start_accept();

//         echo_server.run();
//     });
//     t.detach();
// }

// void HandleWebSocketData(json& data) {
//     std::string event = data["event"];
//     if (event == "set_size") {
//         int w = data["data"]["w"];
//         int h = data["data"]["h"];
//         gWidth = w;
//         gHeight = h;

//         if (gSwapChain) {
//             gContext->OMSetRenderTargets(0, 0, 0);
//             if (gRTV) { gRTV->Release(); gRTV = nullptr; }

//             gSwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

//             ID3D11Texture2D* backBuffer = nullptr;
//             gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
//             gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);
//             backBuffer->Release();

//             gContext->OMSetRenderTargets(1, &gRTV, nullptr);

//             D3D11_VIEWPORT vp = {};
//             vp.TopLeftX = 0;
//             vp.TopLeftY = 0;
//             vp.Width = (float)w;
//             vp.Height = (float)h;
//             gContext->RSSetViewports(1, &vp);
//         }

//         SetWindowPos(gHwnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

//     }
//     else if (event == "set_pos") {
//         int x = data["data"]["x"];
//         int y = data["data"]["y"];
//         SetWindowPos(gHwnd, nullptr, x, y, gWidth, gHeight, SWP_NOZORDER);
//     }
// }

// // ------------------- WinMain -------------------
// int main() {
//     SetProcessDPIAware();

//     HWND parent = FindWindowA(NULL, "clipforge"); // Tauri 窗口

//     WNDCLASS wc = {};
//     wc.hInstance = GetModuleHandle(nullptr);
//     wc.lpszClassName = "window";
//     wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
//         return DefWindowProc(hwnd, msg, wParam, lParam);
//         };
//     wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//     wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

//     if (!RegisterClass(&wc)) {
//         std::cerr << "RegisterClass failed, error = " << GetLastError() << std::endl;
//         return -1;
//     }

//     gHwnd = CreateWindowA(
//         "window",
//         "MyRenderer",
//         WS_POPUP | WS_VISIBLE, // 无边框
//         0, 0, gWidth, gHeight,
//         parent, NULL, wc.hInstance, NULL
//     );

//     if (!gHwnd) {
//         std::cerr << "CreateWindow failed, error = " << GetLastError() << std::endl;
//         return -1;
//     }

//     if (!InitD3D(gHwnd, gWidth, gHeight)) {
//         std::cerr << "D3D11 Init Failed\n";
//         return -1;
//     }

//     // 启动渲染线程
//     std::thread(RenderLoop).detach();

//     // 保留你的 WebSocket 初始化
//     init_websocket([=](json& data) {
//         HandleWebSocketData(data);
//     });

//     ShowWindow(gHwnd, SW_SHOW);
//     UpdateWindow(gHwnd);

//     MSG msg = {};
//     while (GetMessage(&msg, nullptr, 0, 0)) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     return 0;
// }
#include "application.h"
#include "Dx11Renderer.h"

int main()
{
    Application app;
    Dx11Renderer renderer(800, 600, app.hwnd_);
    app.run(&renderer);
    return 0;
}