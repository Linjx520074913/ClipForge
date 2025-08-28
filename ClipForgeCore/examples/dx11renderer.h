#pragma once

#include "renderer.h"
#include <d3d11.h>
#include <wrl/client.h>  // ComPtr 

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

/**
 *  Win32 窗口
    ↓
    D3D11CreateDeviceAndSwapChain()
    ↓
    [设备]       [上下文]      [交换链]
    ↓            ↓             ↓
    创建资源   绑定资源+绘制   管理后台缓冲
    ↓
    渲染循环：
    1. ClearRenderTargetView()
    2. Draw()
    3. swapChain->Present()
    ↓
    屏幕显示
 */

class Dx11Renderer : public IRenderer
{
public:
    Dx11Renderer(int w, int h, HWND hwnd) : IRenderer(w, h, hwnd) {}
    
    void init();
    void render();
    void destroy();

protected:
    void init_buffer();
    void init_shader();

private:
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> ctx_;
    ComPtr<IDXGISwapChain> swap_chain_;
    ComPtr<ID3D11RenderTargetView> rtv_;

    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    ComPtr<ID3D11InputLayout> input_layout_;
};