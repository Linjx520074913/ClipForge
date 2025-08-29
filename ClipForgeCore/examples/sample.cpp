#include "application.h"
#include "dx11renderer.h"
#include "decoder.h"

#include <string>
#include <iostream>

int main()
{
    SetProcessDPIAware();

    Application app;
    app.set_ws_callback([](json& data){
        const std::string event = data["data"]["event"];
        if(event == "set_size") {
            
        }
    });
    Dx11Renderer dx11(800, 600, app.hwnd_);
    dx11.init();
    
    Decoder decoder;
    decoder.open_video("D://video//video.mp4", [&](AVFrame* frame){
        
        if(frame->format == AV_PIX_FMT_D3D11) {
            ID3D11Texture2D* t_frame = (ID3D11Texture2D*)frame->data[0];
            int t_index = (int)frame->data[1];

            ComPtr<ID3D11Device> device;
            t_frame->GetDevice(device.GetAddressOf());

            ComPtr<ID3D11DeviceContext> ctx;
            device->GetImmediateContext(ctx.GetAddressOf());

            // 初始化阶段就 OpenSharedResource, 这里直接用 ComPtr 持有即可
            ComPtr<ID3D11Texture2D> shared;
            device->OpenSharedResource(dx11.get_shared_texture(), __uuidof(ID3D11Texture2D), (void**)shared.GetAddressOf());

            // 拷贝数据到渲染共享纹理
            ctx->CopySubresourceRegion(shared.Get(), 0, 0, 0, 0, t_frame, t_index, 0);
            ctx->Flush();

            // 调用渲染
            dx11.render();
        }

        
    });

    app.run();
    return 0;
}