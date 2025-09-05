#pragma once
#include "renderer.h"
#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct Transform {
    DirectX::XMMATRIX transform;
};

class Dx11Renderer : public IRenderer
{
public:
    Dx11Renderer(int w, int h, HWND hwnd) : IRenderer(w, h, hwnd) {
        init();
    }

    /**
     * 渲染硬解码帧
     */
    void render_hardware_frame(ID3D11Texture2D* frame, int t_index);
    /**
     * 渲染软解码帧
     */
    void render_software_frame();

    void update_transform(float tx, float ty, float scale, float angle, float win_w, float win_h, float video_w, float video_h);
    
    void render();
    void destroy();
    void resize();

protected:
    void init();
    void init_vertex_buffer();
    void init_constant_buffer();
    void init_sampler();
    void init_shader();
    void init_texture(int w, int h);

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
    
    ID3D11ShaderResourceView* y_srv_ = nullptr;
    ID3D11ShaderResourceView* uv_srv_ = nullptr;

    ID3D11Texture2D* texture_ = nullptr;
    HANDLE shared_tex_handle_ = nullptr;

    ID3D11Buffer* transform_;

    bool need_resize_;
};
