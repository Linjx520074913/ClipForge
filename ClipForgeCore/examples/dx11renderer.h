#pragma once
#include "renderer.h"
#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

class Dx11Renderer : public IRenderer
{
public:
    Dx11Renderer(int w, int h, HWND hwnd) : IRenderer(w, h, hwnd) {}

    void init();
    void render();
    void destroy();
    void resize();

    // 上传 AVFrame 的 Y 分量
    void updateYTexture(uint8_t* y_data, int width, int height);

    HANDLE get_shared_texture() { return shared_tex_handle_; }

protected:
    void init_buffer();
    void init_shader();
    void init_texture();
    void init_sampler();

private:
    ID3D11Device1* device_ = nullptr;
    ID3D11DeviceContext1* ctx_ = nullptr;
    IDXGISwapChain1* swap_chain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;

    ID3D11VertexShader* v_shader_ = nullptr;
    ID3D11PixelShader* p_shader_ = nullptr;
    ID3D11InputLayout* input_layout_ = nullptr;

    ID3D11Buffer* v_buffer_ = nullptr;
    UINT num_ = 0, stride_ = 0, offset_ = 0;

    ID3D11SamplerState* sampler_state_ = nullptr;
    ID3D11ShaderResourceView* texture_srv_ = nullptr;

    ID3D11Texture2D* texture_ = nullptr;
    HANDLE shared_tex_handle_ = nullptr;
};
