struct RenderUnit {

}

impl RenderUnit {
    
}

struct Compositor {

}

impl Compositor {
    
}

struct Engine {
    deivce: wgpu::Device,
    queue:  wgpu::Queue,

    surface: wgpu::Surface,
    config:  wgpu::SurfaceConfiguration,

    track_renderers: Vec<RenderUnit>,

    compositor: Compositor
}

impl Engine {
    // async fn new(window: &winit::window::Window) -> Self {
    //     let size = window.inner_size();
        
    //     let instance = wgpu::Instance::new(wgpu::Backends::all());
    //     let surface = instance.create_surface(window).unwrap();
    //     let adapter = instance
    //         .request_adapter(&wgpu::RequestAdapterOptions {
    //             power_preference: wgpu::PowerPreference::HighPerformance,
    //             compatible_surface: Some(&surface),
    //             force_fallback_adapter: false
    //         })
    //         .await
    //         .expect("Failed to find an appropriate adapter");
        
    //     let (device, queue) = adapter
    //         .request_device(
    //         &wgpu::DeviceDescriptor {
    //                 label: None,
    //                 features: wgpu::Features::empty(),
    //                 limits: wgpu::Limits::default()
    //             }, None,
    //         )
    //         .await
    //         .expect("Failed to create device");

    //     let surface_format = surface.get_supported_formats(&adapter)[0];
    //     let config = wgpu::SurfaceConfiguration {
    //         usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
    //         format: surface_format,
    //         width: size.width,
    //         height: size.height,
    //         present_mode: wgpu::PresentMode::Fifo,
    //         alpha_mode: wgpu::CompositeAlphaMode::Auto,
    //     }; 
    //     Self {

    //     }
    // }
}