<template>
    <div
        ref="rootRef"
        class='bg-blue-300 relative'
    >
        <!-- 预览区域 -->
        <canvas ref="canvasRef"
            @click="isFocusd = true"/>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineEmits, onMounted, onBeforeUnmount, ref } from "vue";
import {
    useMP4,
    useClipEngine,
    VideoTrack
} from './index';

const emit    = defineEmits(['ready', 'play', 'pause', 'stop', 'ended', 'error', 'mousedown']);
defineOptions({ name: 'VideoPreview' });
const props   = defineProps({
    size:{
        w: { type: Number, default: 100 },
        h: { type: Number, default: 100 }
    },
    src: { type: String, default: ''}
});

let { engine, videoTrack, initClipEngine } = useClipEngine();

let { player } = useMP4();

const canvasRef = ref<HTMLCanvasElement | null>(null);

function play(){
    if(!player) return;
    player.play();
}

function pause(){
    if(!player) return;
    player.pause();
}

function stop(){
    if(!player) return;
    player.stop();
}

onMounted(async () => {
    engine = await initClipEngine();
    videoTrack = new VideoTrack("video-track-0", engine.getContext(), canvasRef.value);
    engine.addTrack(videoTrack);

    // 监听 ready 事件，获取视频尺寸
    player.on("ready", ({ width, height }) => {
        
        canvasRef.value.width = width;
        canvasRef.value.height = height;
    });

    // 监听 updateFrame 事件，拿到每一帧 VideoFrame
    player.on("updateFrame", (frame: VideoFrame) => {
        engine.render(frame);
        // frame 不能长期保存，处理完应手动 close
        frame.close();
    });

    // 加载视频
    await player.load(props.src);
    
    
});

defineExpose({
    play,
    pause,
    stop
})
</script>