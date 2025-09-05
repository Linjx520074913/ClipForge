#include "application.h"
#include "dx11renderer.h"
#include "decoder.h"

#include <string>
#include <iostream>

#include "timeline_clock.h"

int main()
{
    Application app;
    Dx11Renderer dx11(800, 600, app.hwnd_);
    
    Decoder decoder;
    decoder.open_video("D://video//video.mp4", [&](AVFrame* frame){
        
        if(frame->format == AV_PIX_FMT_D3D11) {
            ID3D11Texture2D* t_frame = (ID3D11Texture2D*)frame->data[0];
            int t_index = (int)frame->data[1];

            dx11.render_hardware_frame(t_frame, t_index);
        }  
    });

    TimelineClock clock;
    clock.set_tick([](double time){
        std::cout << "############# " << time << " ########### " << std::endl;
    });
    clock.play();

    app.run(&dx11);
    return 0;
}