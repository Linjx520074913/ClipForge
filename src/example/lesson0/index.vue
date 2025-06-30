<template>
    <div>
        <h1>WebGPU 绘制红色三角形</h1>
        <canvas ref="canvasRef" class="w-[600px] h-[600px] border border-red-500"></canvas>
    </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';

const canvasRef = ref<HTMLCanvasElement | null>(null);
onMounted(async () => {
    const canvas = canvasRef.value;
    if(!canvas) return;

    // 获取可用显卡
    const adapter = await navigator.gpu?.requestAdapter();
    // 获取显卡控制器
    const device = await adapter?.requestDevice();
    if(!device){
        console.error('WebGPU not supported or device not available');
        return;
    }

    const context = canvas.getContext('webgpu') as any; 
    // 获取显示最佳纹理格式
    const format = navigator.gpu.getPreferredCanvasFormat();

    context.configure({
        device,
        format,
        alphaMode: 'opaque',
    })

    // 创建着色器模块
    const shaderModule = device.createShaderModule({
        label: 'Triangle Shader',
        code:`
            @vertex fn vs(@builtin(vertex_index) vertexIndex: u32)
                ->@builtin(position) vec4f{
                let pos = array<vec2f, 3>(
                    vec2f(-0.5, -0.5),
                    vec2f(0.5, -0.5),
                    vec2f(0.0, 0.5)
                );
                return vec4f(pos[vertexIndex], 0.0, 1.0);
            }

            @fragment fn fs() -> @location(0) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0); // 红色
            }
        `
    })

    // 创建渲染管线
    // 渲染管线是 WebGPU 中的核心概念，用于描述如何渲染图形
    const pipleline = device.createRenderPipeline({
        layout: 'auto',
        vertex:{
            module: shaderModule,
            entryPoint: 'vs',
        },
        fragment:{
            module: shaderModule,
            entryPoint: 'fs',
            targets: [{
                format,
            }],
        },
        primitive:{
            topology: 'triangle-list',
        }
    });

    const draw = () => {
        // 创建命令编码器
        // 命令编码器用于记录渲染命令
        const commandEncoder = device.createCommandEncoder();
        // 创建渲染通道
        // 渲染通道用于描述如何渲染到画布上
        const renderPass = commandEncoder.beginRenderPass({
            colorAttachments: [{
                view: context.getCurrentTexture().createView(),
                loadOp: 'clear',
                storeOp: 'store',
                clearValue: { r: 0.5, g: 0.1, b: 0.8, a: 0.5 }, // 清除为黑色
            }],
        })

        renderPass.setPipeline(pipleline);
        renderPass.draw(3)
        renderPass.end()
        device.queue.submit([commandEncoder.finish()]);
    }

    draw()
})
</script>