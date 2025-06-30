<template>
    <div>
        <h1>WebGPU 图像处理（反色）</h1>
        <canvas ref="canvasRef" class="w-[600px] h-[600px] border border-red-500"></canvas>
        <video ref="videoRef" src="@assets/test2.mp4" muted playsinline controls/>       
    </div>
</template>
  
<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
import mosaicShader from '../shader/mosaic.wgsl?raw';
import waveShader from '../shader/wave.wgsl?raw';

const canvasRef = ref<HTMLCanvasElement | null>(null);
const videoRef = ref<HTMLVideoElement | null>(null);

let adapter: GPUAdapter | null = null;
let device: GPUDevice | null = null;
let context: GPUCanvasContext | null = null;
let pipeline: GPURenderPipeline | null = null;
let sampler: GPUSampler | null = null;
let texture: GPUTexture | null = null;
let uniformBuffer: GPUBuffer | null = null;
let bindGroup: GPUBindGroup | null = null;

async function initWebGPU(width: number, height: number) {
    adapter = await navigator.gpu.requestAdapter({
        // powerPreference: 'high-performance'
    });
    if (!adapter) throw new Error('WebGPU adapter not found');

    device = await adapter.requestDevice();
    if (!device) throw new Error('WebGPU device not found');

    const canvas = canvasRef.value!;
    context = canvas.getContext('webgpu')!;
    const format = navigator.gpu.getPreferredCanvasFormat();

    canvas.width = width;
    canvas.height = height;

    context.configure({
        device,
        format,
        alphaMode: 'opaque',
    });

    // 创建纹理，大小同视频帧
    texture = device.createTexture({
        size: [width, height, 1],
        format: 'rgba8unorm',
        usage:
        GPUTextureUsage.TEXTURE_BINDING |
        GPUTextureUsage.COPY_DST |
        GPUTextureUsage.RENDER_ATTACHMENT,
    });

    sampler = device.createSampler({
        magFilter: 'linear',
        minFilter: 'linear',
    });

    uniformBuffer = device.createBuffer({
        size: 8, // 2 floats: width, height
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    const module = device.createShaderModule({ code: waveShader });

    pipeline = device.createRenderPipeline({
        layout: 'auto',
        vertex: {
        module,
        entryPoint: 'vs_main',
        },
        fragment: {
        module,
        entryPoint: 'fs_main',
        targets: [{ format }],
        },
        primitive: {
        topology: 'triangle-list',
        },
    });

    bindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [
        { binding: 0, resource: sampler },
        { binding: 1, resource: texture.createView() },
        { binding: 2, resource: { buffer: uniformBuffer } },
        ],
    });
}

async function cleanupWebGPU(){
    uniformBuffer?.destroy?.();
    texture?.destroy?.()
    sampler = null
    pipeline = null
    bindGroup = null
    context = null
    device = null
    adapter = null
}

// 逐帧渲染函数
async function renderFrame() {
    if (!videoRef.value || !device || !context || !pipeline || !texture || !uniformBuffer || !bindGroup) return;

    const video = videoRef.value;

    if (video.readyState < 2) {
        // 视频未就绪，等待下一帧
        requestAnimationFrame(renderFrame);
        return;
    }

    // 拷贝视频当前帧到GPU纹理
    const frame = new VideoFrame(video);
    device.queue.copyExternalImageToTexture(
        { source: frame },
        { texture },
        [ video.videoWidth, video.videoHeight ]
    );
    frame.close();

    // 更新uniform buffer（canvas大小）
    device.queue.writeBuffer(uniformBuffer, 0, new Float32Array([video.videoWidth, video.videoHeight]));

    // 开始渲染
    const encoder = device.createCommandEncoder();
    const pass = encoder.beginRenderPass({
        colorAttachments: [{
            view: context.getCurrentTexture().createView(),
            loadOp: 'clear',
            storeOp: 'store',
            clearValue: { r: 0, g: 0, b: 0, a: 1 },
        }],
    });

    pass.setPipeline(pipeline);
    pass.setBindGroup(0, bindGroup);
    pass.draw(6);
    pass.end();

    device.queue.submit([encoder.finish()]);

    requestAnimationFrame(renderFrame);
}

onMounted(() => {
    const video = videoRef.value!;
    const canvas = canvasRef.value!;
    video.addEventListener('loadedmetadata', async () => {
        // 初始化WebGPU资源
        console.error('Video metadata loaded:', video.videoWidth, video.videoHeight);
        await initWebGPU(video.videoWidth, video.videoHeight);

        // 启动渲染循环
        requestAnimationFrame(renderFrame);
    });
});

onUnmounted(() => {
    cleanupWebGPU()
})
</script>

<style scoped>
canvas {
  width: 100%;
  height: auto;
  max-width: 3840px;
  max-height: 2160px;
  background-color: black;
}
</style>