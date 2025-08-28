#include "dx11renderer.h"
#include <vector>
#include <iostream>

void Dx11Renderer::init()
{
    // 1、准备交换链描述（窗口大小、格式、缓冲区数量等）。
    DXGI_SWAP_CHAIN_DESC sc_desc = {};
    auto& buf_desc = sc_desc.BufferDesc;
    buf_desc.Width = width_;                        // 渲染缓冲区分辨率
    buf_desc.Height = height_;
    buf_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;   // 像素格式
    buf_desc.RefreshRate.Numerator = 0;             // 使用显示器默认刷新率
    buf_desc.RefreshRate.Denominator = 0;
    buf_desc.Scaling = DXGI_MODE_SCALING_STRETCHED; // 图像缩放方式，这里是拉伸
    buf_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 扫描方式

    sc_desc.SampleDesc.Count = 1;                   // 不适用多重采样（无抗锯齿）
    sc_desc.SampleDesc.Quality = 0;                 // 质量级别，多重采样时生效

    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 缓冲区作为渲染目标
    sc_desc.BufferCount = 2;                        // 双缓冲

    sc_desc.OutputWindow = hwnd_;                   // 渲染输出到这个窗口
    sc_desc.Windowed = true;                        // 窗口模式，不全屏

    sc_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // 前后缓冲交互策略，推荐，性能高
    sc_desc.Flags = 0;

    // D3D11_CREATE_DEVICE_DEBUG 只能在安装调试层的系统中使用，正式版本要去掉
    // UINT flags = D3D11_CREATE_DEVICE_DEBUG;
    UINT flags = 0;

    D3D_FEATURE_LEVEL level;

    // 2、调用 D3D11CreateDeviceAndSwapChain 得到
    // - ID3DDevice       : 设备对象，用于创建资源
    // - ID3DDeviceContext: 上下文，用于绘制
    // - IDXGISwapChain   ：交换链，用于显示
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &sc_desc,
        &swap_chain_,
        &device_,
        &level,
        &ctx_
    );

    if (FAILED(hr)) {
        std::cerr << "D3D11CreateDeviceAndSwapChain failed: 0x" << std::hex << hr << std::endl;
        return;
    }

    // 3、获取后台缓冲
    ComPtr<ID3D11Texture2D> back_buffer;
    swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)back_buffer.GetAddressOf());

    // 4、创建 RenderTargetView
    device_->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_.GetAddressOf());

    // 5、绑定到管线
    ctx_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);

    init_buffer();
    init_shader();
}

void Dx11Renderer::init_buffer()
{
    // 1、顶点输入
    struct Vertex { float x; float y; float z; };
    const Vertex vertices[] = {
        { -1, 1, 0 },
        { 1, 1, 0 },
        { 1, -1, 0 },
        { -1, -1, 0 }
    };
    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    bd.StructureByteStride = sizeof(Vertex);
    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    ComPtr<ID3D11Buffer> vBuffer;
    device_->CreateBuffer(&bd, &sd, &vBuffer);

    UINT stride = sizeof(Vertex);
    UINT offset = 0u;
    ID3D11Buffer* vBuffers[] = { vBuffer.Get() };
    ctx_->IASetVertexBuffers(0, 1, vBuffers, &stride, &offset);

    // 2、顶点索引
    const UINT16 indices[] = { 0, 1, 2, 0, 2, 3 };
    auto indicesSize = std::size(indices);
    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.ByteWidth = sizeof(indices);
    ibd.StructureByteStride = sizeof(UINT16);
    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices;

    ComPtr<ID3D11Buffer> iBuffer;
    device_->CreateBuffer(&ibd, &isd, &iBuffer);
    ctx_->IASetIndexBuffer(iBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // 告诉系统我们画的是三角形
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Dx11Renderer::init_shader()
{
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    ComPtr<ID3DBlob> errorBlob;

    // Vertex Shader
    HRESULT hr = D3DCompile(
        R"(
        struct VSInput { float3 pos : POSITION; };
        struct PSInput { float4 pos : SV_POSITION; };
        PSInput VSMain(VSInput input) { PSInput o; o.pos = float4(input.pos,1.0); return o; }
        )",
        strlen(R"(
        struct VSInput { float3 pos : POSITION; };
        struct PSInput { float4 pos : SV_POSITION; };
        PSInput VSMain(VSInput input) { PSInput o; o.pos = float4(input.pos,1.0); return o; }
        )"),
        nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0,
        &vsBlob, &errorBlob
    );
    if (FAILED(hr)) { std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl; }

    // Pixel Shader
    hr = D3DCompile(
        R"(
        struct PSInput { float4 pos : SV_POSITION; };
        float4 PSMain(PSInput input) : SV_TARGET { return float4(0,1,0,1); } 
        )",
        strlen(R"(
        struct PSInput { float4 pos : SV_POSITION; };
        float4 PSMain(PSInput input) : SV_TARGET { return float4(0,1,0,1); } 
        )"),
        nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0,
        &psBlob, &errorBlob
    );
    if (FAILED(hr)) { std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl; }

    // 创建 Shader
    device_->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, vs_.GetAddressOf());
    device_->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, ps_.GetAddressOf());

    // 输入布局
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    device_->CreateInputLayout(layout, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), input_layout_.GetAddressOf());
}

void Dx11Renderer::render()
{
    // 清屏 - 绑定资源 - 设置着色器 - 绘制 - 显示
    float clearColor[4] = { 0, 1, 0, 1 }; // 黑色背景
    ctx_->ClearRenderTargetView(rtv_.Get(), clearColor);

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = width_;
    vp.Height = height_;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    ctx_->RSSetViewports(1, &vp);

    // 绑定 Shader
    ctx_->IASetInputLayout(input_layout_.Get());
    ctx_->VSSetShader(vs_.Get(), nullptr, 0);
    ctx_->PSSetShader(ps_.Get(), nullptr, 0);

    // 绘制
    ctx_->DrawIndexed(6, 0, 0);

    // 显示
    swap_chain_->Present(1, 0);

    
}
void Dx11Renderer::destroy()
{

}