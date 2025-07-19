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
import { ClipEngine, ShaderDescription } from "clip-engine";

import { v4 as uuidv4 } from 'uuid';

const emit    = defineEmits(['ready', 'play', 'pause', 'stop', 'ended', 'error', 'mousedown']);
defineOptions({ name: 'VideoPreview' });
const props = defineProps<{
    size:{
        w: { type: Number, default: 100 },
        h: { type: Number, default: 100 }
    },
    src: { type: String, default: '' },
    engine: { type: ClipEngine }
}>();

let { videoTrack, initClipEngine } = useClipEngine();

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

function addEffect(desc: ShaderDescription){
    console.error('############## addEffect', videoTrack, (videoTrack as VideoTrack).getEffectChain());
    (videoTrack as VideoTrack).getEffectChain().addNode(desc);
}

function removeEffect(desc: ShaderDescription){
    (videoTrack as VideoTrack).getEffectChain().removeNode(desc.name);
}

onMounted(async () => {
    // engine = await initClipEngine();
    const id = uuidv4();
    videoTrack = new VideoTrack(`video-track-${id}`, props.engine.getContext(), canvasRef.value);
    // props.engine.addTrack(videoTrack);

    // 监听 ready 事件，获取视频尺寸
    player.on("ready", ({ width, height }) => {
        
        canvasRef.value.width = width;
        canvasRef.value.height = height;
    });

    // 监听 updateFrame 事件，拿到每一帧 VideoFrame
    player.on("updateFrame", (frame: VideoFrame) => {
        videoTrack.render(frame);
        // frame 不能长期保存，处理完应手动 close
        frame.close();
    });

    // 加载视频
    await player.load(props.src);
});

defineExpose({
    play,
    pause,
    stop,
    addEffect,
    removeEffect
})
</script>