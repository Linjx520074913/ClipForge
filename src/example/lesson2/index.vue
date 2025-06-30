<template>
    <div class="flex flex-col space-y-2">
        <div class="w-[600px] h-[400px] border border-red-500 grid place-items-center">
            <h>原始视频</h>
            <video ref="videoRef" src="@assets/test2.mp4" class="rounded-xl" muted playsinline controls/>
        </div>
        <div class="flex flex-row space-x-2">
            <div class="w-[600px] h-[400px] border border-red-500 grid place-items-center">
                <h1>WebGPU 波纹</h1>
                <canvas ref="canvasRef" class="rounded-xl"/>
            </div>
            <div class="w-[600px] h-[400px] border border-red-500 grid place-items-center">
                <h1> CanvasRenderer </h1>
                <canvas ref="canvasRendererRef" class="rounded-xl"/>
            </div>
        </div>
    </div>
</template>
  
<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
import { WebGPURenderer, CanvasRenderer, createPlayer } from '@avcore';

const canvasRendererRef = ref<HTMLCanvasElement | null>(null);
const canvasRef = ref<HTMLCanvasElement | null>(null);
const videoRef = ref<HTMLVideoElement | null>(null);

onMounted(async () => {
    
    const canvas = canvasRef.value!;

    // 添加 WebGPURenderer 的状态，好了之后 player 才播放
    const canvas0Renderer = new WebGPURenderer(canvas)
    const player0 = createPlayer('webav', canvas0Renderer);
    await player0.load('./test2.mp4')
    player0.play()

    // const canvas1Renderer = new CanvasRenderer(canvasRef.value!)
    // const player1 = createPlayer('webav', canvas1Renderer);
    // await player1.load('./test2.mp4')
    // player1.play()
});

onUnmounted(() => {
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