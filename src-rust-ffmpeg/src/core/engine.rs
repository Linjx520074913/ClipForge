use wgpu::{
    Device, Queue, Surface, SurfaceConfiguration, 
    Instance, InstanceDescriptor, Backends, InstanceFlags, MemoryBudgetThresholds,
    BackendOptions, RequestAdapterOptions, PowerPreference,
    DeviceDescriptor, Features, Limits, MemoryHints, Trace, TextureUsages, PresentMode
};
use winit::window::Window;
use std::{ sync::Arc };
use indexmap::IndexMap;

use image::{GenericImageView, ImageBuffer, Rgba};

use super::renderer;

pub struct RenderUnit;
pub struct Compositor;

pub struct Engine {
    device: Arc<Device>,
    queue:  Arc<Queue>,

    surface: Surface<'static>,
    config:  SurfaceConfiguration,

    track_renderers: Vec<RenderUnit>,

    // scene_renderer: RenderUnit

    compositor: Compositor
}

impl Engine {

    pub async fn new(window: &Window) -> Self {
        let size = window.inner_size();
        
        let instance = Instance::new(&InstanceDescriptor {
            backends: Backends::all(),
            flags:    InstanceFlags::empty(),
            memory_budget_thresholds: MemoryBudgetThresholds::default(),
            backend_options: BackendOptions::default()
        });
        let surface = instance.create_surface(window).unwrap();
        // 安全地扩展生命周期为 'static
        let surface = unsafe {
            std::mem::transmute::<Surface<'_>, Surface<'static>>(surface)
        };

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
        // let surface_format = surface_caps.formats.iter()
        //     .copied()
        //     .find(|f| f.is_srgb())
        //     .unwrap_or(surface_caps.formats[0]);
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
     
        let img = image::open("E://lion.jpg").expect("Failed to open image");
        let (width, height) = img.dimensions();
        let rgba = img.to_rgba8();

        let texture_size = wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        };

        // 3. 创建输入纹理
        let input_texture = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("Input Texture"),
            size: texture_size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
            view_formats: &[],
        });

        queue.write_texture(
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
        
        // let entries = IndexMap::from([
        //     (String::from("param1"), renderer::ShaderParam {
        //         value: 1.0,
        //         label: String::from("Param 1"),
        //         min: 0.0,
        //         max: 10.0,
        //         step: 0.1
        //     }),
        //     (String::from("param2"), renderer::ShaderParam {
        //         value: 1.0,
        //         label: String::from("Param 2"),
        //         min: 0.0,
        //         max: 10.0,
        //         step: 0.1
        //     }),
        //     (String::from("param3"), renderer::ShaderParam {
        //         value: 1.0,
        //         label: String::from("Param 3"),
        //         min: 0.0,
        //         max: 10.0,
        //         step: 0.1
        //     })
        // ]);
        let shader_desc = renderer::ShaderDescriptor {
            id: Some(String::from("Id")),
            name: String::from("Test"),
            code: String::from(include_str!("../shaders/RawShader.wgsl")),
            params: renderer::ShaderParamPack {
                binding: 2,
                entries: IndexMap::new(),
                runtime: vec!["update".to_string()]
            },
            enabled: true,
        };
        let test = renderer::Renderer::new(device.clone(), queue.clone(), &shader_desc);
        let frame = surface.get_current_texture().unwrap();
        let surface_view = frame.texture.create_view(&wgpu::TextureViewDescriptor::default());

        test.process(&input_texture, &surface_view);
        frame.present();

        Self {
            device,
            queue,
            surface,
            config,
            track_renderers: Vec::new(),
            compositor: Compositor
        }
    }
}