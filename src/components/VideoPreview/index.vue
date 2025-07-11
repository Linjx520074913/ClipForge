<template>
    <div
        ref="rootRef"
        class='bg-red-300 relative'
    >
        <!-- 预览区域 -->
        <canvas ref="previewCanvasRef"
            @click="isFocusd = true"/>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, onMounted, ref, defineExpose, onBeforeUnmount } from 'vue';
import { VideoPlayer } from '@avcore';
import throttle from 'lodash/throttle';
import { IFilter } from 'src/modules/av-core/renderer/Filter/Type';

defineOptions({ name: 'VideoPreview' });
const props = defineProps({
    size:{
        w: { type: Number, default: 100 },
        h: { type: Number, default: 100 }
    },
    src: { type: String, default: ''}
})

const emit = defineEmits(['ready', 'play', 'pause', 'stop', 'ended', 'error', 'mousedown']);

const previewCanvasRef = ref<HTMLCanvasElement | null>(null);
let player = ref<VideoPlayer | null>(null);

const rootRef = ref<HTMLDivElement | null>(null);

const percent = ref(0);

const curTimeUs  = ref(0);

const seekable = ref(false);
const isDragging = ref(false);

// 预览区域是否聚焦
const isFocusd = ref(false);


function handleClick(type: 'play' | 'pause' | 'stop') {
    if (!player.value){
        return;
    }

    switch (type) {
        case 'play':
            player.value.play();
            break;
        case 'pause':
            player.value.pause();
            break;
        case 'stop':
            player.value.stop();
            break;
        default:
            break;
    }
    emit(type, {})
}

/**
 * 聚焦
 */
function focusOn(){

}

// 格式化时间
function formatTime(time: number) {
    if (time <= 0) {
        return '00:00:00.000';
    }

    const totalMs = Math.floor(time / 1000); // 微秒转毫秒
    const ms = totalMs % 1000;
    const totalSec = Math.floor(totalMs / 1000);
    const h = Math.floor(totalSec / 3600);
    const m = Math.floor((totalSec % 3600) / 60);
    const s = totalSec % 60;

    const hh = String(h).padStart(2, '0');
    const mm = String(m).padStart(2, '0');
    const ss = String(s).padStart(2, '0');
    const mmm = String(ms).padStart(3, '0');

    return `${hh}:${mm}:${ss}.${mmm}`;
}


onMounted(async () => {
    if (!previewCanvasRef.value) return;

    rootRef.value.style.width = `${props.size.w}px`;
    rootRef.value.style.height = `${props.size.h}px`;
    
    try {
        player.value = new VideoPlayer(previewCanvasRef.value, props.size);
        
        // 注册播放器事件
        player.value.on('ready', () => {
            emit('ready');
        });
        
        player.value.on('play', () => {
            emit('play');
        });

        player.value.on('updateFrame', () => {
            if (!isDragging.value) {
                curTimeUs.value = player.value.getCurrentTimeUs();
                percent.value = curTimeUs.value / player.value.getDurationUs() * 100;
            } else {
                curTimeUs.value = player.value.getCurrentTimeUs();
            }
        })
        
        player.value.on('pause', () => {
            emit('pause');
        });
        
        player.value.on('stop', () => {
            emit('stop');
        });
        
        player.value.on('ended', () => {
            emit('ended');
        });
        
        player.value.on('error', () => {
            emit('error');
        });
        
        // 加载视频源
        if (props.src) {
            await player.value.load(props.src);
        }
        // player.value.play();
    } catch (error) {
        console.error('MediaPlayer initialization failed:', error);
        emit('error');
    }
});

function play(){
    if(!player.value) return;

    player.value.play();
}

function pause(){
    if(!player.value) return;

    player.value.pause();
}

function stop(){
    if(!player.value) return;

    player.value.stop();
}

function setFilters(filters: IFilter[]){
    console.error('========== 设置滤镜', filters, player.value);
    if(!player.value) return;
    console.error('========== +++++++++');
    player.value.setFilters(filters);
}

onBeforeUnmount(() => {
    if (player.value) {
        player.value.destroy();
    }
});

defineExpose({
    play,
    pause,
    stop,
    setFilters
});
</script>