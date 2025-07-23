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
    VideoTrack,
    useWebAV
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

let videoTrack: VideoTrack;

let { loadMediaSource, seek, setVideoTrack } = useWebAV();

const canvasRef = ref<HTMLCanvasElement | null>(null);

function addEffect(desc: ShaderDescription){
    (videoTrack as VideoTrack).getEffectChain().addNode(desc);
}

function removeEffect(desc: ShaderDescription){
    (videoTrack as VideoTrack).getEffectChain().removeNode(desc.name);
}

function updateEffect(desc: ShaderDescription){
    (videoTrack as VideoTrack).getEffectChain().getNode(desc.name)?.applyParameters(desc.params);
}

onMounted(async () => {
    const id = uuidv4();
    videoTrack = new VideoTrack(`video-track-${id}`, props.engine.getContext(), canvasRef.value);
    setVideoTrack(videoTrack);

    // 加载视频
    const { duration, width, height } =await loadMediaSource(props.src);
    canvasRef.value.width = width;
    canvasRef.value.height = height;

    seek(0);
    
});

defineExpose({
    seek,
    addEffect,
    removeEffect,
    updateEffect
})
</script>