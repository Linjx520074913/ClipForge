<template>
    <div class="flex flex-col space-y-2">
        <MediaPlayer :width="650" :height="450" :src="src"/>
    </div>
</template>
  
<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';
import { MediaPlayer } from '@components/index';

const src = ref<string>('./test2.mp4')

onMounted(async () => {
    
    // const canvas = canvasRef.value!;

    // // 添加 WebGPURenderer 的状态，好了之后 player 才播放
    // const canvas0Renderer = new WebGPURenderer(canvas)
    // const player0 = createPlayer('webav', canvas0Renderer);
    // await player0.load('./test2.mp4')
    // player0.play()

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