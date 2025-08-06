use crate::core::renderer_unit::RendererUnit;

use super::renderer_unit;

use std::sync::Mutex;
use tauri::{window, WebviewWindow};
use wgpu::{util::DeviceExt, Limits};

use std::sync::Arc;


pub struct Engine<'win> {
    pub device: Arc<wgpu::Device>,
    pub queue: Arc<wgpu::Queue>,

    pub surface: wgpu::Surface<'win>,

    pub unit: RendererUnit
    
}

impl Engine<'_> {
    pub async fn init(window: WebviewWindow) -> Self {
        let size = window.inner_size().unwrap();
        print!("init window size = {} x {}", size.width, size.height);

        let instance = wgpu::Instance::default();
        let surface = instance.create_surface(window).unwrap();
        let adapter = instance
            .request_adapter(&wgpu::RequestAdapterOptions {
                power_preference: wgpu::PowerPreference::HighPerformance,
                force_fallback_adapter: false,
                compatible_surface: Some(&surface)
            })
            .await
            .expect("Failed to find an appropriate adapter");
        print!("Find adapter");

        let limits = wgpu::Limits {
            max_storage_buffers_per_shader_stage: 8,
            ..Default::default()
        };

        let (device, queue) = adapter
            .request_device(
                &wgpu::DeviceDescriptor {
                    label: None,
                    required_features: wgpu::Features::empty(),
                    required_limits: limits
                }, None)
            .await
            .expect("Failed to create device");
        print!("Create device");

        let device = Arc::new(device);
        let queue = Arc::new(queue);

        let shader_code: &str = include_str!("shaders/mosaic.wgsl");
        let size = 10.0;
        let unit = RendererUnit::new(device.clone(), queue.clone(), shader_code, size);

        Self {
            device,
            queue,
            surface,
            unit
        }
    }

    pub fn render(time_ms: i32) {
        print!("render");
    }
}