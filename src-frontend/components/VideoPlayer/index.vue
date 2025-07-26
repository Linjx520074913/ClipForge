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

let videoTrack: VideoTrack;

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

defineExpose({
    addEffect,
    removeEffect,
    updateEffect
})
</script>