<template>
    <div class="p-4">
        <p>Test ClipEngine</p>
        <div class="flex flex-row space-x-4 mt-8">
            <div>
                <p>InputTexture</p>
                <img
                    ref="imgRef" 
                    class="border border-red-500 w-[400px] h-[400px] object-contain"
                    src="/imgs/test-lion.jpg"/>
            </div>
            <div class="w-[200px]">
                滤镜列表
                <ul>
                    <li v-for="(f, index) in filters" :key="index" class="w-full bg-gray-300 mb-1 p-2">
                        <label class="font-bold">{{ f.name }}</label>
                        <div v-for="(param, key) in f.params.entries" :key="key" class="flex flex-col mt-2">
                            <!-- f32 单值 -->
                            <label class="text-sm">{{ key }}</label>
                            <input
                                v-if="param.type === 'f32'"
                                type="range"
                                step="0.01"
                                min="0"
                                max="1"
                                v-model.number="param.value"
                                @input="onParamsChange(f.params)"
                                class="w-full"
                            />
                            <span class="text-xs text-gray-500">{{ param.value }}</span>
                        </div>
                    </li>
                </ul>
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
import { ref, onMounted, onUnmounted } from 'vue';

import { ClipEngine, VideoTrack } from 'clip-engine';

// 添加 VideoFrame 类型声明
declare global {
    interface Window {
        VideoFrame: any;
    }
}

const imgRef = ref<HTMLImageElement | null>(null);
const canvasRef = ref<HTMLCanvasElement | null>(null);

const filters = ref<any[]>([]);
const memoryUsage = ref<number>(0);

async function loadFilters(){
    const filters = [];
    const list = [
        { 
            name: 'cartoon',
            codeURL: './shader/cartoon/cartoon.wgsl',
            code: '',
            paramsURL: '/shader/cartoon/params.json',
            params: { }
        },
        { 
            name: 'cartoon',
            codeURL: './shader/cartoon/cartoon.wgsl',
            code: '',
            paramsURL: '/shader/cartoon/params.json',
            params: { }
        }
    ]
    for(const f of list){
        const code = await (await fetch('/shader/cartoon/cartoon.wgsl')).text();
        const params = JSON.parse(await (await fetch('/shader/cartoon/params.json')).text());
        filters.push({ name: f.name, code, params })
    }

    return filters;
}

let engine: ClipEngine | null = null;
let track: TrackRenderer | null = null;


let CartoonShaderCode: string;
let CartoonParams: any;
let videoFrame: any = null; // 使用any类型避免VideoFrame构造函数问题
let isRendering = false; // 添加渲染状态标志
let isInitialized = false; // 添加初始化状态标志

// 防抖函数，避免频繁渲染
let renderTimeout: number | null = null;


async function onParamsChange(params: any){
    
}

onMounted(async () => {
    try {
        const loadedFilters = await loadFilters();
        filters.value = loadedFilters;
       
        if(!imgRef.value) {
            console.error('Image ref is null');
            return;
        }
        
        const bitmap = await createImageBitmap(imgRef.value);
        if (canvasRef.value && imgRef.value) {
            canvasRef.value.width = imgRef.value.naturalWidth;
            canvasRef.value.height = imgRef.value.naturalHeight;
        }
        
        // 转换为 VideoFrame
        videoFrame = new (window as any).VideoFrame(bitmap, {
            timestamp: performance.now(),
            duration: 0 // 无持续时间
        });

        CartoonShaderCode = await (await fetch('/shader/cartoon/cartoon.wgsl')).text();
        CartoonParams = JSON.parse(await (await fetch('/shader/cartoon/params.json')).text());
        
        if(!canvasRef.value) {
            console.error('Canvas ref is null');
            return;
        }

        engine = await ClipEngine.create();

        track = new VideoTrack('video-track-0', engine.getContext(), canvasRef.value);
        engine.addTrack(track);
        
        await engine.render(videoFrame);
       
    } catch (error) {
    }
});

// 组件卸载时清理资源
onUnmounted(() => {
});
</script>