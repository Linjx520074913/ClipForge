use indexmap::IndexMap;
use wgpu::{
    BackendOptions, Backends, Device, DeviceDescriptor, Features, Instance, InstanceDescriptor, InstanceFlags, Limits, MemoryBudgetThresholds, MemoryHints, PowerPreference, PresentMode, Queue, RequestAdapterOptions, Surface, SurfaceConfiguration, Texture, TextureUsages, TextureView, Trace
};
use tauri::WebviewWindow;
use std::{ sync::Arc };

use image::{ GenericImageView };

use super::renderer::{ Renderer, ShaderDescriptor, ShaderParamPack, ShaderParam };
use super::cf_decoder::{ CFDecoder, CFMetadata };

pub struct RenderUnit;
pub struct Compositor;

pub struct Engine {
    pub device: Arc<Device>,
    pub queue:  Arc<Queue>,

    pub surface: Surface<'static>,
    config:  SurfaceConfiguration,

    track_renderers: Vec<RenderUnit>,

    scene_renderer: Option<Renderer>,

    compositor: Compositor,

    input_texture: Option<wgpu::Texture>,

    pub decoder: CFDecoder,

    pub start_time: Option<std::time::Instant>
}


impl Engine {

    pub async fn new(window: WebviewWindow) -> Self {

        let decoder = CFDecoder::new();
        // decoder.open_video("D://test.mp4");
        decoder.open_video("D://video//Q360_20250612_141124_000001.MP4");

        let size = window.inner_size().unwrap();
        
        let instance = wgpu::Instance::default();
        let surface = instance.create_surface(window).unwrap();

        let adapter = instance
            .request_adapter(&RequestAdapterOptions {
                power_preference: PowerPreference::HighPerformance,
                compatible_surface: Some(&surface),
                force_fallback_adapter: false
            })
            .await
            .expect("Failed to find an appropriate adapter");
        
        let (device, queue) = adapter
            .request_device(
            &DeviceDescriptor {
                    label: Some("Engine Device"),
                    required_features: Features::empty(),
                    required_limits: Limits::default(),
                    memory_hints: MemoryHints::default(),   // 内存使用提示
                    trace: Trace::default()                 // 用于 GPU 调试追踪
                },
            )
            .await
            .expect("Failed to create device");
        
        let device = Arc::new(device);
        let queue = Arc::new(queue);
        let surface_caps = surface.get_capabilities(&adapter);
        let surface_format = wgpu::TextureFormat::Rgba8UnormSrgb;

        let config = SurfaceConfiguration {
            usage: TextureUsages::RENDER_ATTACHMENT,
            format: surface_format,
            width: size.width,
            height: size.height,
            present_mode: PresentMode::Fifo,
            alpha_mode: surface_caps.alpha_modes[0],               // 取默认的透明度模式
            view_formats: vec![],
            desired_maximum_frame_latency: 2                       // 性能与延迟的调节开关
        };

        // 格式为 Bgra8UnormSrgb
        surface.configure(&device, &config);
        
        log::info!("Engine initialized: {}x{}, format: {:?}", size.width, size.height, surface_format);

        let scene_renderer = None;
        let input_texture = None;

        let start_time = None;

        let mut engine = Self {
            device,
            queue,
            surface,
            config,
            track_renderers: Vec::new(),
            scene_renderer,
            compositor: Compositor,
            input_texture,
            decoder,
            start_time
        };

        // engine.initialize_input_texture();
        engine.initialize_scene_renderer();
        engine
    }

    /**
     * 初始化场景渲染器，经过 engine 处理后的图像最终由 scene_renderer 渲染到屏幕
     */
    fn initialize_scene_renderer(&mut self) {

        // 场景渲染器使用的是 rawshader 直接输出图像，不需要额外的 params
        let entries = IndexMap::from([
            (String::from("width"), ShaderParam {
                value: self.config.width as f32,
                label: String::from("width"),
                min: 0.0,
                max: 20.0,
                step: 0.1
            }),
            (String::from("height"), ShaderParam {
                value: self.config.height as f32,
                label: String::from("height"),
                min: 0.0,
                max: 20.0,
                step: 0.1
            }),
        ]);
        self.scene_renderer = Some(Renderer::new(
            self.device.clone(),
            self.queue.clone(),
            &ShaderDescriptor {
                id: Some(String::from("Id")),
                name: String::from("Scene Renderer"),
                code: String::from(include_str!("./shaders/RawShader.wgsl")),
                params: ShaderParamPack {
                    binding: 2,
                    entries: entries,
                    runtime: vec!["update".to_string()]
                },
                enabled: true,
            }
        ));
    }

    /**
     * 初始化输入纹理
     */
    fn initialize_input_texture(&mut self) {
        let img = image::open("E://123.jpg").expect("Failed to open image");
        let (width, height) = img.dimensions();
        let rgba = img.to_rgba8();

        let texture_size = wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        };

        let input_texture = self.device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Input Texture"),
            size: texture_size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
            view_formats: &[],
        });

        self.queue.write_texture(
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

    pub fn create_texture(&self, width: u32, height: u32, data: &[u8]) -> Texture{
        let texture_size = wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        };

        let input_texture = self.device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Input Texture"),
            size: texture_size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
            view_formats: &[],
        });

        self.queue.write_texture(
            wgpu::TexelCopyTextureInfo  {
                texture: &input_texture,
                mip_level: 0,
                origin: wgpu::Origin3d::ZERO,
                aspect: wgpu::TextureAspect::All,
            },
            &data,
            wgpu::TexelCopyBufferLayout  {
                offset: 0,
                bytes_per_row: Some(4 * width),
                rows_per_image: Some(height),
            },
            texture_size,
        );

        input_texture
    }

    pub fn resize(&mut self, w: u32, h: u32) {
        if w > 0 && h > 0 {
            self.config.width  = w;
            self.config.height = h;
            self.surface.configure(&self.device, &self.config);
        }
    }

    /**
     * 渲染
     */
    pub fn render_frame(&mut self) {

        if(self.start_time.is_none()) {
            self.start_time = Some(std::time::Instant::now());
        }

        let timestamp = self.start_time.unwrap().elapsed().as_millis() as i64;
        let frame_ptr = self.decoder.get_current_frame(timestamp);
        if frame_ptr.is_null() {
            return;
        }

        unsafe {
            let frame_ref = &*frame_ptr;
            let data_ptr: *const u8 = frame_ref.data;
            let len = frame_ref.length as usize;

            let data_slice: &[u8] = std::slice::from_raw_parts(data_ptr, len);
            let input_texture = self.create_texture(frame_ref.width, frame_ref.height, data_slice);
        
            match self.surface.get_current_texture() {
                Ok(frame) => {
                    let surface_view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());

                    let scene_renderer = self.scene_renderer.as_mut().unwrap();
                    scene_renderer.set_param_value("width", self.config.width as f32);
                    scene_renderer.set_param_value("height", self.config.height as f32);
                    scene_renderer.process(&input_texture, &surface_view);

                    frame.present();

                    self.decoder.free_frame(frame_ptr);
                }
                Err(wgpu::SurfaceError::Outdated) | Err(wgpu::SurfaceError::Lost) => {
                    // 重建 surface
                }
                Err(wgpu::SurfaceError::OutOfMemory) => {
                    panic!("Swap chain out of memory");
                }
                Err(e) => {
                    eprintln!("Dropped frame : {:?}", e);
                }
            }

            
        }

    }
}