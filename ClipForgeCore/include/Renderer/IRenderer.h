#pragma once
#include <windows.h>

struct AVFrame;

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    /**
     * 初始化渲染器（例如绑定窗口）
     */
    virtual bool init(HWND hwnd, int width, int height) = 0;

    /**
     * 渲染一帧 AVFrame 数据
     */
    virtual void render_frame(AVFrame* frame) = 0;

    /**
     * 窗口尺寸变化
     */
    virtual void resize(int width, int height) = 0;

    /**
     * 清理资源
     */
    virtual void destroy() = 0;
};