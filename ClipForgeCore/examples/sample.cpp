#include <windows.h>
#include <iostream>

// 窗口回调函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int main()
{
    // 1.1 注册窗口类
    WNDCLASS wc = {};
    // wc.lpfnWndProc = WndPro
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "window";
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if(!RegisterClass(&wc)) {
        std::cerr << "RegisterClass failed, error = " << GetLastError() << std::endl;
        return -1;
    }

    // 1.2 创建窗口
    HWND hwnd = CreateWindow(
        wc.lpszClassName,              // 类名
        "FFmpeg - Dx9 demo",           // 窗口标题
        WS_OVERLAPPEDWINDOW,           // 风格
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,                      // 宽高
        nullptr, nullptr,
        wc.hInstance, nullptr
    );

    if(!hwnd) {
        std::cerr << "CreateWindow failed, error = " << GetLastError() << std::endl;
        return -1;
    }

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