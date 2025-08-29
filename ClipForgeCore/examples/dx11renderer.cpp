#include "Dx11Renderer.h"
#include "start.h"  // 里面有 STAR_RGBA_DATA
#include <iostream>

#include <assert.h>

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
}

void Dx11Renderer::init_buffer() {
    struct Vertex { float x, y, z; float u, v; };
    const Vertex vertices[] = {
        { -1,  1, 0, 0, 0 },
        {  1,  1, 0, 1, 0 },
        {  1, -1, 0, 1, 1 },
        { -1, -1, 0, 0, 1 }
    };
    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    D3D11_SUBRESOURCE_DATA sd = { vertices };
    device_->CreateBuffer(&bd, &sd, &v_buffer_);

    stride_ = sizeof(Vertex);
    offset_ = 0;

    const UINT16 indices[] = { 0, 1, 2, 0, 2, 3 };
    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(indices);
    D3D11_SUBRESOURCE_DATA isd = { indices };
    device_->CreateBuffer(&ibd, &isd, &i_buffer_);
}

void Dx11Renderer::init_shader() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    // Vertex Shader
    const char* vsSrc = R"(
    struct VSInput { float3 pos : POSITION; float2 uv : TEXCOORD; };
    struct PSInput { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };
    PSInput VSMain(VSInput input) {
        PSInput o; 
        o.pos = float4(input.pos, 1.0); 
        o.uv = input.uv; 
        return o; 
    }
    )";

    HRESULT hr = D3DCompile(
        vsSrc, strlen(vsSrc),   // 或者直接 -1
        nullptr, nullptr, nullptr,
        "VSMain", "vs_5_0", 0, 0,
        &vsBlob, &errorBlob
    );
    if (FAILED(hr)) std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;

    // Pixel Shader
    const char* psSrc = R"(
    Texture2D tex : register(t0);
    SamplerState samLinear : register(s0);

    struct PSInput { float4 pos : SV_POSITION; float2 uv : TEXCOORD; };

    float4 PSMain(PSInput input) : SV_TARGET {
        return tex.Sample(samLinear, input.uv);
    }
    )";

    hr = D3DCompile(
        psSrc, strlen(psSrc),            // 或者 strlen(psSrc)
        nullptr, nullptr, nullptr,
        "PSMain", "ps_5_0",
        0, 0,
        &psBlob, &errorBlob
    );
    if (FAILED(hr)) std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;

    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs_.GetAddressOf());
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps_.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    device_->CreateInputLayout(layout, ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        input_layout_.GetAddressOf());
}

void Dx11Renderer::init_texture() {

    D3D11_TEXTURE2D_DESC tdesc = {};
    tdesc.Width = 32;
    tdesc.Height = 32;
    tdesc.MipLevels = 1;
    tdesc.ArraySize = 1;
    tdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tdesc.SampleDesc.Count = 1;
    tdesc.Usage = D3D11_USAGE_DEFAULT;
    tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = STAR_RGBA_DATA;
    initData.SysMemPitch = 32 * 4;

    device_->CreateTexture2D(&tdesc, &initData, &texture);
    device_->CreateShaderResourceView(texture.Get(), nullptr, &texture_srv_);
}

void Dx11Renderer::init_sampler() {
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    device_->CreateSamplerState(&sampDesc, sampler_.GetAddressOf());
}

void Dx11Renderer::render()
{
    FLOAT bg_color[4] = { 0.1, 0.2, 0.6, 1.0 };
    ctx_->ClearRenderTargetView(rtv_, bg_color);

    swap_chain_->Present(1, 0);
}

void Dx11Renderer::destroy() {}
