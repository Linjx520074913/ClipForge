use winit::application::ApplicationHandler;
use winit::event::WindowEvent;
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::window::{Window, WindowId};

mod core;
use core::engine::Engine;
use core::renderer::{
    Renderer, ShaderDescriptor, ShaderParam, ShaderParamPack
};

use indexmap::IndexMap;

use image::{GenericImageView};

use std::time::Instant;


#[derive(Default)]
struct App {
    window: Option<Window>,
    engine: Option<Engine>,
    value : f32,
    input_texture: Option<wgpu::Texture>
}

impl ApplicationHandler for App {
    /**
     * 应用恢复，创建窗口
     */
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        let window = event_loop.create_window(Window::default_attributes()).unwrap();
        
        let engine = pollster::block_on(Engine::new(&window));

        self.window = Some(window);
        self.engine = Some(engine);
        self.value  = 0.0;

        let img = image::open("E://123.jpg").expect("Failed to open image");
        let (width, height) = img.dimensions();
        let rgba = img.to_rgba8();

        let texture_size = wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        };

        let engine_ref = self.engine.as_ref().unwrap();
        
        // 3. 创建输入纹理
        let input_texture = engine_ref.device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Input Texture"),
            size: texture_size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
            view_formats: &[],
        });

        engine_ref.queue.write_texture(
            wgpu::TexelCopyTextureInfo  {
                texture: &input_texture,
                mip_level: 0,
                origin: wgpu::Origin3d::ZERO,
                aspect: wgpu::TextureAspect::All,
            },
            &rgba,
            wgpu::TexelCopyBufferLayout  {
                offset: 0,
                bytes_per_row: Some(4 * width),
                rows_per_image: Some(height),
            },
            texture_size,
        );
        self.input_texture = Some(input_texture);

    }

    /**
     * 监听窗口事件
     */
    fn window_event(&mut self, event_loop: &ActiveEventLoop, id: WindowId, event: WindowEvent) {
        match event {
            WindowEvent::CloseRequested => {
                println!("The close button was pressed; stopping");
                event_loop.exit();
            },
            WindowEvent::RedrawRequested => {
                
                let engine = self.engine.as_ref().unwrap();
                self.value += 0.04;

                let entries = IndexMap::from([
                    (String::from("size"), ShaderParam {
                        value: self.value,
                        label: String::from("size"),
                        min: 0.0,
                        max: 20.0,
                        step: 0.1
                    })
                ]);
                let shader_desc = ShaderDescriptor {
                    id: Some(String::from("Id")),
                    name: String::from("Test"),
                    code: String::from(include_str!("./shaders/mosaic.wgsl")),
                    params: ShaderParamPack {
                        binding: 2,
                        // entries: IndexMap::new(),
                        entries: entries,
                        runtime: vec!["update".to_string()]
                    },
                    enabled: true,
                };
                let test = Renderer::new(engine.device.clone(), engine.queue.clone(), &shader_desc);
                let frame = engine.surface.get_current_texture().unwrap();
                let surface_view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());

                if let Some(ref input_texture) = self.input_texture {
                    test.process(input_texture, &surface_view);
                } else {
                    eprintln!("Error: input_texture not initialized");
                }
                frame.present();

                // Redraw the application.
                //
                // It's preferable for applications that do not render continuously to render in
                // this event rather than in AboutToWait, since rendering in here allows
                // the program to gracefully handle redraws requested by the OS.

                // Draw.

                // Queue a RedrawRequested event.
                //
                // You only need to call this if you've determined that you need to redraw in
                // applications which do not always need to. Applications that redraw continuously
                // can render here instead.
                self.window.as_ref().unwrap().request_redraw();
            }
            _ => (),
        }
    }
}

fn init_env_logger() {
    env_logger::Builder::new()
        .filter_level(log::LevelFilter::Info)
        .init();
    log::info!("Starting application : init_env_logger done");
}

fn main() {
    init_env_logger();

    let event_loop = EventLoop::new().unwrap();

    // ControlFlow::Poll continuously runs the event loop, even if the OS hasn't
    // dispatched any events. This is ideal for games and similar applications.
    event_loop.set_control_flow(ControlFlow::Poll);

    let mut app = App::default();
    event_loop.run_app(&mut app).expect("Failed to run app");
}