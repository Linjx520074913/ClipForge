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

import { ClipEngine, FilterNode, TrackRenderer } from 'clip-engine';
import CartoonShaderCode from './cartoon.wgsl?raw';


const imgRef = ref<HTMLImageElement | null>(null);
const canvasRef = ref<HTMLCanvasElement | null>(null);

onMounted(async () => {
    let engine = new ClipEngine();
    await engine.init();

    let track0 = new TrackRenderer('video-track-0', canvasRef.value);
    engine.addTrackRenderer(track0);

    const bitmap = await createImageBitmap(imgRef.value);
  
    // 转换为 VideoFrame
    const videoFrame = new VideoFrame(bitmap, {
        timestamp: performance.now(),
        duration: 0 // 无持续时间
    });

    if (canvasRef.value && imgRef.value) {
        canvasRef.value.width = imgRef.value.naturalWidth;
        canvasRef.value.height = imgRef.value.naturalHeight;
    }

    const filter = new FilterNode(CartoonShaderCode, 'cartoon');
    
    track0.filterPipeline?.addFilterNode(filter);
    filter.applyParams();
    track0.render(videoFrame);

    
});
</script>