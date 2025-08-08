use crate::core::renderer_unit::RendererUnit;

use super::renderer_unit;

use std::sync::Mutex;
use tauri::{window, WebviewWindow};
use wgpu::{util::DeviceExt, Limits};

use std::sync::Arc;

use image::{GenericImageView, ImageBuffer, Rgba};
use futures_intrusive::channel::shared::oneshot_channel;


fn align_to(value: u32, alignment: u32) -> u32 {
    ((value + alignment - 1) / alignment) * alignment
}

pub struct Engine<'win> {
    pub device: Arc<wgpu::Device>,
    pub queue: Arc<wgpu::Queue>,

    pub surface: wgpu::Surface<'win>,

    pub unit: RendererUnit
    
}

impl Engine<'_> {
    pub async fn new(window: WebviewWindow) -> Self {
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

        let img = image::open("E://test.jpg").expect("打开图片失败").to_rgba8();
        let (width, height) = img.dimensions();
        let size = wgpu::Extent3d {
            width,
            height,
            depth_or_array_layers: 1,
        };
        let input_texture = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("InputTexture"),
            size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::COPY_DST | wgpu::TextureUsages::TEXTURE_BINDING,
            view_formats: &[],
        });
        queue.write_texture(
            wgpu::ImageCopyTexture {
                texture: &input_texture,
                mip_level: 0,
                origin: wgpu::Origin3d::ZERO,
                aspect: wgpu::TextureAspect::All,
            },
            &img,
            wgpu::ImageDataLayout {
                offset: 0,
                bytes_per_row: Some(4 * width),
                rows_per_image: Some(height),
            },
            size,
        );
    
        // 4. 创建输出纹理（render target）
        let output_texture = device.create_texture(&wgpu::TextureDescriptor {
            label: Some("OutputTexture"),
            size,
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT | wgpu::TextureUsages::COPY_SRC,
            view_formats: &[],
        });

        unit.process(&input_texture, &output_texture);
        // 8. 处理 COPY_BYTES_PER_ROW_ALIGNMENT 对齐
    const COPY_BYTES_PER_ROW_ALIGNMENT: u32 = 256;
    let bytes_per_pixel = 4;
    let unaligned_bytes_per_row = width * bytes_per_pixel;
    let aligned_bytes_per_row = align_to(unaligned_bytes_per_row, COPY_BYTES_PER_ROW_ALIGNMENT);

    let buffer_size = (aligned_bytes_per_row * height) as wgpu::BufferAddress;

    // 9. 创建缓冲区用来读取结果
    let output_buffer = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("Output Buffer"),
        size: buffer_size,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });

    // 10. 创建命令编码器，拷贝纹理到缓冲区
    let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("Readback Encoder"),
    });

    encoder.copy_texture_to_buffer(
        wgpu::ImageCopyTexture {
            texture: &output_texture,
            mip_level: 0,
            origin: wgpu::Origin3d::ZERO,
            aspect: wgpu::TextureAspect::All,
        },
        wgpu::ImageCopyBuffer {
            buffer: &output_buffer,
            layout: wgpu::ImageDataLayout {
                offset: 0,
                bytes_per_row: Some(aligned_bytes_per_row),
                rows_per_image: Some(height),
            },
        },
        size,
    );

    queue.submit(Some(encoder.finish()));

    // 11. 等待映射完成
    let buffer_slice = output_buffer.slice(..);
    let (sender, receiver) = oneshot_channel();
    buffer_slice.map_async(wgpu::MapMode::Read, move |v| sender.send(v).unwrap());
    device.poll(wgpu::Maintain::Wait);
    receiver.receive().await.unwrap().unwrap();

    // 12. 读取数据，去除行尾对齐填充
    let data = buffer_slice.get_mapped_range();

    // 克隆数据，断开对映射内存的引用
    let mut pixels = Vec::with_capacity((width * height * bytes_per_pixel) as usize);
    for chunk in data.chunks(aligned_bytes_per_row as usize) {
        pixels.extend_from_slice(&chunk[..(unaligned_bytes_per_row as usize)]);
    }

    // drop 映射引用，确保没有持有映射内存的引用
    drop(data);

    // 13. 解除映射
    output_buffer.unmap();

    // 14. 转换为 ImageBuffer 并保存
    let img_buffer =
        ImageBuffer::<Rgba<u8>, _>::from_raw(width, height, pixels).expect("创建图像失败");
    img_buffer.save("output.png").expect("保存图片失败");

    println!("马赛克处理完成，结果保存在 output.png");

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