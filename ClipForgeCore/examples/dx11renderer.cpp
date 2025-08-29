#include "Dx11Renderer.h"// 里面有 STAR_RGBA_DATA
#include <iostream>

#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEBUG_BUILD

void Dx11Renderer::init() {
    // Create D3D11 Device and Context
    ID3D11Device* base_device;
    ID3D11DeviceContext* base_ctx;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #if defined(DEBUG_BUILD)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    HRESULT hr = D3D11CreateDevice(
        0,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        flags,
        levels,
        ARRAYSIZE(levels),
        D3D11_SDK_VERSION,
        &base_device,
        0,
        &base_ctx
    );
    if(FAILED(hr)) {
        MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
        return;
    }

    hr = base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&device_);
    assert(SUCCEEDED(hr));
    base_device->Release();

    hr = base_ctx->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&ctx_);
    assert(SUCCEEDED(hr));
    base_ctx->Release();

    // Create Swap Chain
    IDXGIFactory2* factory;
    {
        IDXGIDevice1* dxgi_device;
        hr = device_->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device);
        assert(SUCCEEDED(hr));

        IDXGIAdapter* dxgi_adapter;
        hr = dxgi_device->GetAdapter(&dxgi_adapter);
        assert(SUCCEEDED(hr));
        dxgi_device->Release();

        DXGI_ADAPTER_DESC adapter_desc;
        dxgi_adapter->GetDesc(&adapter_desc);
        OutputDebugStringA("Graphics Device : ");
        OutputDebugStringW(adapter_desc.Description);
        hr = dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);
        assert(SUCCEEDED(hr));
        dxgi_adapter->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
    swap_chain_desc.Width  = 0;
    swap_chain_desc.Height = 0;
    swap_chain_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.SampleDesc.Quality = 0;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount = 2;
    swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swap_chain_desc.Flags = 0;

    hr = factory->CreateSwapChainForHwnd(device_, hwnd_, &swap_chain_desc, 0, 0, &swap_chain_);
    assert(SUCCEEDED(hr));
    factory->Release();

    // Create FrameBuffer RenderTarget
    {
        ID3D11Texture2D* frame_buffer;
        hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buffer);
        assert(SUCCEEDED(hr));

        hr = device_->CreateRenderTargetView(frame_buffer, 0, &rtv_);
        assert(SUCCEEDED(hr));
        frame_buffer->Release();
    }

    init_shader();
}

void Dx11Renderer::init_buffer() {
    
}

void Dx11Renderer::init_shader()
{
    // create vertex shader
    ID3DBlob* vs_blob;
    HRESULT hr;
    {
        ID3DBlob* shader_compile_errors_blob;
        hr = D3DCompileFromFile(L"D:/ClipForge/ClipForgeCore/examples/shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vs_blob, &shader_compile_errors_blob);
        if(FAILED(hr)) {
            const char* msg = NULL;
            if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
                msg = "Could not compile shader, file not found";
            } else if(shader_compile_errors_blob) {
                msg = (const char*)shader_compile_errors_blob->GetBufferPointer();
                shader_compile_errors_blob->Release();
            }
            MessageBoxA(0, msg, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return;
        }

        hr = device_->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &v_shader_);
        assert(SUCCEEDED(hr));
    }

    // create pixel shader
    {
        ID3DBlob* p_blob;
        ID3DBlob* shader_compile_errors_blob;
        hr = D3DCompileFromFile(L"D:/ClipForge/ClipForgeCore/examples/shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &p_blob, &shader_compile_errors_blob);
        if(FAILED(hr)) {
            const char* msg = NULL;
            if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
                msg = "Could not compile shader, file not found";
            } else if(shader_compile_errors_blob) {
                msg = (const char*)shader_compile_errors_blob->GetBufferPointer();
                shader_compile_errors_blob->Release();
            }
            MessageBoxA(0, msg, "Shader Compiler Error", MB_ICONERROR | MB_OK);
            return;
        }

        hr = device_->CreatePixelShader(p_blob->GetBufferPointer(), p_blob->GetBufferSize(), nullptr, &p_shader_);
        assert(SUCCEEDED(hr));
        p_blob->Release();
    }

    // Create Input layout
    {
        D3D11_INPUT_ELEMENT_DESC desc[] = 
        {
            { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = device_->CreateInputLayout(desc, ARRAYSIZE(desc), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &input_layout_);
        assert(SUCCEEDED(hr));
        vs_blob->Release();
    }

    // Create Vertex Buffer
    {
        // x y u v
        float data[] = {
            -1, 1, 0, 0,
            1, -1, 1, 1,
            -1, -1, 0, 1,
            -1, 1, 0, 0,
            1, 1, 1, 0,
            1, -1, 1, 1
        };
        stride_ = 4 * sizeof(float);
        num_ = sizeof(data) / stride_;
        offset_ = 0;

        // 以下代码：我要在 GPU 上创建一块固定大小，不可修改，专门用来存顶点数据的缓冲区
        D3D11_BUFFER_DESC v_buffer_desc = {};
        v_buffer_desc.ByteWidth = sizeof(data);
        v_buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;    // 创建后不可修改，GPU 访问最快
        v_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // 标记这是顶点缓冲区，GPU 会用它作为输入装配器读取顶点
        
        D3D11_SUBRESOURCE_DATA v_sub_data = { data };
        hr = device_->CreateBuffer(&v_buffer_desc, &v_sub_data, &v_buffer_);
        assert(SUCCEEDED(hr));
    }

    // Create Samler State
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampler_desc.BorderColor[0] = 1.0f;
    sampler_desc.BorderColor[1] = 1.0f;
    sampler_desc.BorderColor[2] = 1.0f;
    sampler_desc.BorderColor[3] = 1.0f;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    
    device_->CreateSamplerState(&sampler_desc, &sampler_state_);

    // Load Image
    int width, height, channels, req_comp = 4;
    unsigned char* tex_bytes = stbi_load("D://testTexture.png", &width, &height,&channels, req_comp);
    int bytes_per_row = 4 * width;

    // Create Texture
    D3D11_TEXTURE2D_DESC tex_desc = {};
    tex_desc.Width            = width;
    tex_desc.Height           = height;
    tex_desc.MipLevels        = 1;
    tex_desc.ArraySize        = 1;
    tex_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    tex_desc.SampleDesc.Count = 1;
    tex_desc.Usage            = D3D11_USAGE_IMMUTABLE;
    tex_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA tex_sub_data = {};
    tex_sub_data.pSysMem = tex_bytes;
    tex_sub_data.SysMemPitch = bytes_per_row;

    device_->CreateTexture2D(&tex_desc, &tex_sub_data, &texture_);

    device_->CreateShaderResourceView(texture_, nullptr, &texture_srv_);

    free(tex_bytes);

}

void Dx11Renderer::init_texture() {

   
}

void Dx11Renderer::init_sampler() {
    
}

void Dx11Renderer::resize()
{
    ctx_->OMSetRenderTargets(0, 0, 0);
    rtv_->Release();

    HRESULT res = swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(res));
    
    ID3D11Texture2D* d3d11FrameBuffer;
    res = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
    assert(SUCCEEDED(res));

    res = device_->CreateRenderTargetView(d3d11FrameBuffer, NULL, &rtv_);
    assert(SUCCEEDED(res));
    d3d11FrameBuffer->Release();

}

void Dx11Renderer::render()
{
    FLOAT bg_color[4] = { 0.1, 0.2, 0.6, 1.0 };
    ctx_->ClearRenderTargetView(rtv_, bg_color);

    RECT rect;
    GetClientRect(hwnd_, &rect);
    D3D11_VIEWPORT viewport = { 0, 0, (FLOAT)(rect.right - rect.left), (FLOAT)(rect.bottom - rect.top), 0, 1 };
    ctx_->RSSetViewports(1, &viewport);

    ctx_->OMSetRenderTargets(1, &rtv_, nullptr);

    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->IASetInputLayout(input_layout_);

    ctx_->VSSetShader(v_shader_, nullptr, 0);
    ctx_->PSSetShader(p_shader_, nullptr, 0);

    ctx_->PSSetShaderResources(0, 1, &texture_srv_);
    ctx_->PSSetSamplers(0, 1, &sampler_state_);

    ctx_->IASetVertexBuffers(0, 1, &v_buffer_, &stride_, &offset_);

    ctx_->Draw(num_, 0);
    swap_chain_->Present(1, 0);
}

void Dx11Renderer::destroy() {}
