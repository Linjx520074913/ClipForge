use std::sync::Arc;
use indexmap::IndexMap;
use wgpu::util::DeviceExt;
use bytemuck::cast_slice;

#[derive(Clone, Debug)]
pub struct ShaderParam {
    pub value: f32,
    pub label: String,
    pub min:   f32,
    pub max:   f32,
    pub step:  f32
}

#[derive(Clone, Debug)]
pub struct ShaderParamPack {
    pub binding: u32,
    pub entries: IndexMap<String, ShaderParam>,
    pub runtime: Vec<String>
}

pub struct ShaderDescriptor {
    pub id     : Option<String>,
    pub name   : String,
    pub code   : String,
    pub params : ShaderParamPack,
    pub enabled: bool
}

pub struct Renderer {
    device           : Arc<wgpu::Device>,
    queue            : Arc<wgpu::Queue>,
    render_pipeline  : wgpu::RenderPipeline,
    bind_group_layout: wgpu::BindGroupLayout,
    sampler          : wgpu::Sampler,
    uniform_buffer   : Option<wgpu::Buffer>,
    pub params       : ShaderParamPack
}

impl Renderer {
    pub fn new(
        device: Arc<wgpu::Device>,
        queue:  Arc<wgpu::Queue>,
        shader_desc: &ShaderDescriptor
    ) -> Self {
        let shader_module = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("Renderer Shader"),
            source: wgpu::ShaderSource::Wgsl(shader_desc.code.clone().into())
        });

        let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
            label: Some("Renderer Sample"),
            address_mode_u: wgpu::AddressMode::ClampToEdge,
            address_mode_v: wgpu::AddressMode::ClampToEdge,
            address_mode_w: wgpu::AddressMode::ClampToEdge,
            mag_filter:     wgpu::FilterMode::Linear,
            min_filter:     wgpu::FilterMode::Linear,
            mipmap_filter:  wgpu::FilterMode::Nearest,
            ..Default::default()
        });

        let ShaderParamPack { binding, entries, runtime } = shader_desc.params.clone();
        println!("Renderer params: {:?} {:?} {:?}", entries, binding, runtime);

        let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("Renderer Bind Group Layout"),
            entries: &[
                wgpu::BindGroupLayoutEntry {
                    binding: 0,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                    count: None
                },
                wgpu::BindGroupLayoutEntry {
                    binding: 1,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: wgpu::BindingType::Texture {
                        multisampled: false,
                        view_dimension: wgpu::TextureViewDimension::D2,
                        sample_type: wgpu::TextureSampleType::Float { filterable: true }
                    },
                    count: None
                },
                wgpu::BindGroupLayoutEntry {
                    binding: 2,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: wgpu::BindingType::Buffer {
                        ty: wgpu::BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: None
                    },
                    count: None
                }
            ]
        });

        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("Renderer Pipeline Layout"),
            bind_group_layouts: &[&bind_group_layout],
            push_constant_ranges: &[]
        });

        
        let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("Renderer Render Pipeline"),
            layout: Some(&pipeline_layout),
            vertex: wgpu::VertexState {
                module: &shader_module,
                entry_point: Some("vs_main"),
                buffers: &[],
                compilation_options: Default::default()
            },
            fragment: Some(wgpu::FragmentState {
                module: &shader_module,
                entry_point: Some("fs_main"),
                targets: &[Some(wgpu::ColorTargetState {
                    format: wgpu::TextureFormat::Rgba8UnormSrgb,
                    blend: None,
                    write_mask: wgpu::ColorWrites::ALL
                })],
                compilation_options: Default::default()
            }),
            primitive: wgpu::PrimitiveState::default(),
            depth_stencil: None,
            multisample: wgpu::MultisampleState::default(),
            multiview: None,
            cache: None
        });

        let mut renderer = Self {
            device,
            queue,
            render_pipeline,
            bind_group_layout,
            sampler,
            uniform_buffer: None,
            params: shader_desc.params.clone()
        };

        renderer.update_uniform_buffer();
        renderer
    }

    fn update_uniform_buffer(&mut self) {
        let mut uniform_values = Vec::with_capacity(self.params.entries.len());
        for(_key, param) in self.params.entries.iter() {
            uniform_values.push(param.value);
        }

        if self.uniform_buffer.is_none() {
            if uniform_values.is_empty() {
                // Rust wgpu 中的 bindgrouplayout entries 无法动态添加，现在的方案是默认有 binding 2
                uniform_values.push(0.0);
            }
            let buffer = self.device.create_buffer_init(
                &wgpu::util::BufferInitDescriptor {
                    label: Some("Renderer Uniform Buffer"),
                    contents: bytemuck::cast_slice(&uniform_values),
                    usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST
                }
            );
            self.uniform_buffer = Some(buffer);
        }else {
            self.queue.write_buffer(
                &self.uniform_buffer.as_ref().unwrap(),
                0,
                bytemuck::cast_slice(&uniform_values)
            );
        }
    }

    pub fn set_param_value(&mut self, key: &str, value: f32) {
        if let Some(param) = self.params.entries.get_mut(key) {
            param.value = value;
        }

        self.update_uniform_buffer();
    }

    pub fn process(&self, input: &wgpu::Texture, output: &wgpu::TextureView) {
        let input_view = input.create_view(&wgpu::TextureViewDescriptor::default());

        let bind_group = self.device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("Renderer Bind Group"),
            layout: &self.bind_group_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::Sampler(&self.sampler),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::TextureView(&input_view),
                },
                wgpu::BindGroupEntry {
                    binding: 2,
                    resource: self.uniform_buffer.as_ref().unwrap().as_entire_binding(),
                },
            ],
        });

        let mut encoder = self.device.create_command_encoder(&wgpu::CommandEncoderDescriptor {
            label: Some("Renderer Encoder"),
        });

        {
            let mut pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("RendererUnit Pass"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &output,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(wgpu::Color::BLACK),
                        store: wgpu::StoreOp::Store,
                    },
                    depth_slice: None
                })],
                depth_stencil_attachment: None,
                occlusion_query_set: None,
                timestamp_writes: None,
            });

            pass.set_pipeline(&self.render_pipeline);
            pass.set_bind_group(0, &bind_group, &[]);
            pass.draw(0..6, 0..1); // Assuming a triangle for simplicity
        }

        self.queue.submit(Some(encoder.finish()));
    }
}