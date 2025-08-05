use std::num::NonZeroU32;
use wgpu::util::DeviceExt;
use image::{GenericImageView, ImageBuffer, Rgba};
use pollster::block_on;

fn align_to(value: u32, alignment: u32) -> u32 {
    ((value + alignment - 1) / alignment) * alignment
}

fn main() {
    block_on(run());
}

async fn run() {
    // 1. 初始化 GPU
    let instance = wgpu::Instance::default();
    let adapter = instance
        .request_adapter(&wgpu::RequestAdapterOptions::default())
        .await
        .expect("Failed to find adapter");

    let (device, queue) = adapter
        .request_device(&wgpu::DeviceDescriptor::default(), None)
        .await
        .expect("Failed to create device");

    // 2. 载入图片
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
        wgpu::ImageCopyTexture {
            texture: &input_texture,
            mip_level: 0,
            origin: wgpu::Origin3d::ZERO,
            aspect: wgpu::TextureAspect::All,
        },
        &rgba,
        wgpu::ImageDataLayout {
            offset: 0,
            bytes_per_row: Some(4 * width),
            rows_per_image: Some(height),
        },
        texture_size,
    );

    // 4. 创建输出纹理
    let output_texture = device.create_texture(&wgpu::TextureDescriptor {
        label: Some("Output Texture"),
        size: texture_size,
        mip_level_count: 1,
        sample_count: 1,
        dimension: wgpu::TextureDimension::D2,
        format: wgpu::TextureFormat::Rgba8UnormSrgb,
        usage: wgpu::TextureUsages::RENDER_ATTACHMENT | wgpu::TextureUsages::COPY_SRC,
        view_formats: &[],
    });

    // 5. 创建采样器
    let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
        label: Some("Sampler"),
        mag_filter: wgpu::FilterMode::Nearest,
        min_filter: wgpu::FilterMode::Nearest,
        ..Default::default()
    });

    // 6. 创建 bind group layout 和 bind group
    let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
        label: Some("Bind Group Layout"),
        entries: &[
            wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Texture {
                    multisampled: false,
                    view_dimension: wgpu::TextureViewDimension::D2,
                    sample_type: wgpu::TextureSampleType::Float { filterable: false },
                },
                count: None,
            },
            wgpu::BindGroupLayoutEntry {
                binding: 1,
                visibility: wgpu::ShaderStages::FRAGMENT,
                ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::NonFiltering),
                count: None,
            },
        ],
    });

    let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
        label: Some("Bind Group"),
        layout: &bind_group_layout,
        entries: &[
            wgpu::BindGroupEntry {
                binding: 0,
                resource: wgpu::BindingResource::TextureView(&input_texture.create_view(&Default::default())),
            },
            wgpu::BindGroupEntry {
                binding: 1,
                resource: wgpu::BindingResource::Sampler(&sampler),
            },
        ],
    });

    // 7. WGSL 马赛克 shader
    let shader_source = r#"
        @group(0) @binding(0) var input_texture: texture_2d<f32>;
        @group(0) @binding(1) var input_sampler: sampler;

        struct VertexOutput {
            @builtin(position) position: vec4<f32>,
            @location(0) uv: vec2<f32>,
        };

        @vertex
        fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VertexOutput {
            var positions = array<vec2<f32>, 6>(
                vec2<f32>(-1.0, -1.0),
                vec2<f32>(1.0, -1.0),
                vec2<f32>(-1.0, 1.0),
                vec2<f32>(-1.0, 1.0),
                vec2<f32>(1.0, -1.0),
                vec2<f32>(1.0, 1.0),
            );

            var uvs = array<vec2<f32>, 6>(
                vec2<f32>(0.0, 1.0),
                vec2<f32>(1.0, 1.0),
                vec2<f32>(0.0, 0.0),
                vec2<f32>(0.0, 0.0),
                vec2<f32>(1.0, 1.0),
                vec2<f32>(1.0, 0.0),
            );

            var output: VertexOutput;
            output.position = vec4<f32>(positions[vertex_index], 0.0, 1.0);
            output.uv = uvs[vertex_index];
            return output;
        }

        @fragment
        fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
            let pixel_size = vec2<f32>(0.02, 0.02);
            let uv = input.uv;

            let mosaic_uv = vec2<f32>(
                floor(uv.x / pixel_size.x) * pixel_size.x + pixel_size.x * 0.5,
                floor(uv.y / pixel_size.y) * pixel_size.y + pixel_size.y * 0.5,
            );

            let color = textureSample(input_texture, input_sampler, mosaic_uv);
            return color;
        }
    "#;

    let shader_module = device.create_shader_module(wgpu::ShaderModuleDescriptor {
        label: Some("Mosaic Shader"),
        source: wgpu::ShaderSource::Wgsl(shader_source.into()),
    });

    // 8. 管线布局和渲染管线
    let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
        label: Some("Pipeline Layout"),
        bind_group_layouts: &[&bind_group_layout],
        push_constant_ranges: &[],
    });

    let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
        label: Some("Render Pipeline"),
        layout: Some(&pipeline_layout),
        vertex: wgpu::VertexState {
            module: &shader_module,
            entry_point: "vs_main",
            buffers: &[],
            compilation_options: Default::default(),
        },
        fragment: Some(wgpu::FragmentState {
            module: &shader_module,
            entry_point: "fs_main",
            targets: &[Some(wgpu::ColorTargetState {
                format: wgpu::TextureFormat::Rgba8UnormSrgb,
                blend: Some(wgpu::BlendState::REPLACE),
                write_mask: wgpu::ColorWrites::ALL,
            })],
            compilation_options: Default::default(),
        }),
        primitive: wgpu::PrimitiveState::default(),
        depth_stencil: None,
        multisample: wgpu::MultisampleState::default(),
        multiview: None,
    });

    // 9. 渲染
    let output_view = output_texture.create_view(&Default::default());

    let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("Render Encoder"),
    });

    {
        let mut render_pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
            label: Some("Render Pass"),
            color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                view: &output_view,
                resolve_target: None,
                ops: wgpu::Operations {
                    load: wgpu::LoadOp::Clear(wgpu::Color::BLACK),
                    store: wgpu::StoreOp::Store,
                },
            })],
            depth_stencil_attachment: None,
            occlusion_query_set: None,
            timestamp_writes: None,
        });

        render_pass.set_pipeline(&render_pipeline);
        render_pass.set_bind_group(0, &bind_group, &[]);
        render_pass.draw(0..6, 0..1);
    }

    queue.submit(Some(encoder.finish()));

    // 10. 读取 GPU 纹理到 CPU buffer
    let bytes_per_row_unaligned = 4 * width;
    let bytes_per_row_aligned = align_to(bytes_per_row_unaligned, 256);
    let padded_buffer_size = bytes_per_row_aligned as u64 * height as u64;

    let output_buffer = device.create_buffer(&wgpu::BufferDescriptor {
        label: Some("Output Buffer"),
        size: padded_buffer_size,
        usage: wgpu::BufferUsages::COPY_DST | wgpu::BufferUsages::MAP_READ,
        mapped_at_creation: false,
    });

    let mut encoder = device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
        label: Some("Copy Encoder"),
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
                bytes_per_row: Some(bytes_per_row_aligned),
                rows_per_image: Some(height),
            },
        },
        texture_size,
    );

    queue.submit(Some(encoder.finish()));

    // 11. 映射缓冲区，异步等待完成
    let buffer_slice = output_buffer.slice(..);
    let (sender, receiver) = futures_intrusive::channel::shared::oneshot_channel();
    buffer_slice.map_async(wgpu::MapMode::Read, move |v| sender.send(v).unwrap());

    device.poll(wgpu::Maintain::Wait);

    receiver.receive().await.unwrap().unwrap();

    // 12. 读取数据，跳过padding
    {
        let data = buffer_slice.get_mapped_range();

        let mut pixels = Vec::with_capacity((4 * width * height) as usize);
        for chunk in data.chunks(bytes_per_row_aligned as usize) {
            pixels.extend_from_slice(&chunk[0..(4 * width) as usize]);
        }

        let result_image = ImageBuffer::<Rgba<u8>, _>::from_raw(width, height, pixels)
            .expect("Failed to create image buffer");

        result_image.save("output.png").expect("Failed to save image");
    } // data 离开作用域，确保安全 unmap

    output_buffer.unmap();

    println!("马赛克图片已保存到 output.png");
}
