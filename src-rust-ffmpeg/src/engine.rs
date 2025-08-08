use wgpu::{
    Device, Queue, Surface, SurfaceConfiguration, 
    Instance, InstanceDescriptor, Backends, InstanceFlags, MemoryBudgetThresholds,
    BackendOptions, RequestAdapterOptions, PowerPreference,
    DeviceDescriptor, Features, Limits, MemoryHints, Trace, TextureUsages, PresentMode
};
use winit::window::Window;
pub struct RenderUnit;
pub struct Compositor;

pub struct Engine {
    device: Device,
    queue:  Queue,

    surface: Surface<'static>,
    config:  SurfaceConfiguration,

    track_renderers: Vec<RenderUnit>,

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
        
        let surface_caps = surface.get_capabilities(&adapter);
        let surface_format = surface_caps.formats.iter()
            .copied()
            .find(|f| f.is_srgb())
            .unwrap_or(surface_caps.formats[0]);

        let config = SurfaceConfiguration {
            usage: TextureUsages::RENDER_ATTACHMENT,
            format: surface_format,
            width: size.width,
            height: size.height,
            present_mode: PresentMode::Fifo,
            alpha_mode: surface_caps.alpha_modes[0],               // 取默认的透明度模式
            view_formats: vec![],
            desired_maximum_frame_latency: 2                 // 性能与延迟的调节开关
        };

        surface.configure(&device, &config);

        log::info!("Engine initialized: {}x{}, format: {:?}", size.width, size.height, surface_format);
     
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