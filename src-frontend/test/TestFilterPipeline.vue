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
            <div class="w-[200px]">
                滤镜列表
                <ul>
                    <li v-for="(f, index) in filters" :key="index" class="w-full bg-gray-300 mb-1">
                        <label>{{  f.name }}</label>
                        <div v-for="(param, key) in f.params.entries" :key="key" class="flex flex-row">
                            <!-- f32 单值 -->
                            <label>{{ key }}</label>
                            <input
                                v-if="param.type === 'f32'"
                                type="range"
                                step="0.1"
                                min="0"
                                max="1"
                                v-model.number="param.value"
                                @input="onParamsChange(f.params)"
                            />
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
import { ref, onMounted } from 'vue';

import { ClipEngine, FilterNode, TrackRenderer } from 'clip-engine';

const imgRef = ref<HTMLImageElement | null>(null);
const canvasRef = ref<HTMLCanvasElement | null>(null);

const filters = ref([]);

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

let engine = new ClipEngine();
let track;
let fileterNode: FilterNode;

let CartoonShaderCode;
let CartoonParams;
let videoFrame;

async function onParamsChange(params){
    console.error('=========', params)

    fileterNode.applyParams(params);
    await track.render(videoFrame);

}

onMounted(async () => {
    filters.value = await loadFilters();
   
    const bitmap = await createImageBitmap(imgRef.value);
    if (canvasRef.value && imgRef.value) {
        canvasRef.value.width = imgRef.value.naturalWidth;
        canvasRef.value.height = imgRef.value.naturalHeight;
    }
    // 转换为 VideoFrame
    videoFrame = new VideoFrame(bitmap, {
        timestamp: performance.now(),
        duration: 0 // 无持续时间
    });

    CartoonShaderCode = await (await fetch('/shader/cartoon/cartoon.wgsl')).text();
    CartoonParams = JSON.parse(await (await fetch('/shader/cartoon/params.json')).text());
    await engine.init();

    track = new TrackRenderer('video-track-0', canvasRef.value);
    engine.addTrackRenderer(track);

    fileterNode = new FilterNode(CartoonShaderCode, 'cartoon');

    track.filterPipeline?.addFilterNode(fileterNode);
});
</script>