<template>
    <div class="p-4">
        <p>Test FilterPipeline</p>
        <div class="flex flex-row space-x-4 mt-8">
            <div>
                <p>InputTexture</p>
                <img
                    ref="imgRef" 
                    class="border border-red-500 w-[400px] h-[400px] object-contain"
                    src="/imgs/test-lion.jpg"/>
            </div>
            <div>
                选择 Filter
            </div>
            <div>
                <p>OutputTexture</p>
                <canvas
                    ref="canvasRef"
                    class="border border-red-500 w-[400px] h-[400px]"/>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue';

const imgRef = ref<HTMLImageElement | null>(null);
const canvasRef = ref<HTMLCanvasElement | null>(null);

let device: GPUDevice;
let context: GPUCanvasContext;
let pipeline: GPURenderPipeline;
let sampler: GPUSampler;
let imgTexture: GPUTexture;

const vertexShaderWGSL = `
    @vertex
    fn vs_main(@builtin(vertex_index) VertexIndex : u32)
        -> @builtin(position) vec4<f32> {
        var pos = array<vec2<f32>, 6>(
        vec2<f32>(-1.0, -1.0),
        vec2<f32>( 1.0, -1.0),
        vec2<f32>(-1.0,  1.0),
        vec2<f32>(-1.0,  1.0),
        vec2<f32>( 1.0, -1.0),
        vec2<f32>( 1.0,  1.0));

        return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
    }
`;

const fragmentShaderWGSL = `
    @group(0) @binding(0) var mySampler: sampler;
    @group(0) @binding(1) var myTexture: texture_2d<f32>;

    @fragment
    fn fs_main(@builtin(position) FragCoord : vec4<f32>)
        -> @location(0) vec4<f32> {
        let texSize = textureDimensions(myTexture, 0);
        let uv = FragCoord.xy / vec2<f32>(texSize);
        return textureSample(myTexture, mySampler, uv);
    }
`;

async function initWebGPU() {
    if (!navigator.gpu) {
        console.error('WebGPU not supported!');
        return;
    }

    const adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
        console.error('Failed to get GPU adapter');
        return;
    }
    device = await adapter.requestDevice();

    if (!canvasRef.value) return;
    context = canvasRef.value.getContext('webgpu') as GPUCanvasContext;
    const format = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
        device,
        format,
        alphaMode: 'premultiplied',
    });

    // 创建着色器模块
    const vertexModule = device.createShaderModule({ code: vertexShaderWGSL });
    const fragmentModule = device.createShaderModule({ code: fragmentShaderWGSL });

    // 创建渲染管线
    pipeline = device.createRenderPipeline({
        layout: 'auto',
            vertex: {
            module: vertexModule,
            entryPoint: 'vs_main',
        },
        fragment: {
            module: fragmentModule,
            entryPoint: 'fs_main',
            targets: [{ format }],
        },
        primitive: {
            topology: 'triangle-list',
        },
    });

    sampler = device.createSampler({
        magFilter: 'linear',
        minFilter: 'linear',
    });
}

async function createTextureFromImage() {
    if (!imgRef.value || !device) return;

    const bitmap = await createImageBitmap(imgRef.value);

    if (canvasRef.value && imgRef.value) {
        canvasRef.value.width = imgRef.value.naturalWidth;
        canvasRef.value.height = imgRef.value.naturalHeight;
    }

    imgTexture = device.createTexture({
        size: [bitmap.width, bitmap.height],
        format: 'rgba8unorm',
        usage:
        GPUTextureUsage.TEXTURE_BINDING |
        GPUTextureUsage.COPY_DST |
        GPUTextureUsage.RENDER_ATTACHMENT,
    });

    device.queue.copyExternalImageToTexture(
        { source: bitmap },
        { texture: imgTexture },
        [bitmap.width, bitmap.height]
    );
}

function render() {
    if (!imgTexture || !context) return;

    const commandEncoder = device.createCommandEncoder();
    const textureView = context.getCurrentTexture().createView();

    const renderPass = commandEncoder.beginRenderPass({
        colorAttachments: [
        {
            view: textureView,
            clearValue: { r: 0, g: 0, b: 0, a: 1 },
            loadOp: 'clear',
            storeOp: 'store',
        },
        ],
    });

    renderPass.setPipeline(pipeline);
    // 创建 BindGroup 绑定纹理和采样器
    const bindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
        { binding: 0, resource: sampler },
        { binding: 1, resource: imgTexture.createView() },
        ],
    });
    renderPass.setBindGroup(0, bindGroup);
    renderPass.draw(6);
    renderPass.end();

    device.queue.submit([commandEncoder.finish()]);
}

onMounted(async () => {
    await initWebGPU();
    await createTextureFromImage();
    render();
});
</script>