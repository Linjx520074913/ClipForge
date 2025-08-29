#pragma once
#include <windows.h>

class IRenderer
{
public:
    virtual void init()    = 0;
    virtual void render()  = 0;
    virtual void destroy() = 0;
    virtual void resize()  = 0;

protected:
    IRenderer(int w, int h, HWND hwnd) : width_(w), height_(h), hwnd_(hwnd) {}

protected:
    int width_;
    int height_;
    float ratio_;
    HWND hwnd_;
};