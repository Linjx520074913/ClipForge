<template>
    <div class="flex flex-col" id="timeline">
        <!-- 控制条 -->
        <div class="w-full h-[40px] flex items-center justify-center">
            <!-- 播放暂停按钮 -->
            <span @click="emit('update:playing', !playing)"
                class="material-icons text-[40px] cursor-pointer hover:scale-110 transition-all duration-250 hover:text-purple">
                {{ playing ? 'pause_circle' : 'play_circle' }}
            </span>
            <div class="text-[12px]">{{ curTimeFormatted }} / 0:00:00</div>
        </div>
        <div class="w-full h-[280px] flex flex-row pl-[10px]">
            <!-- 右边时间轴部分 -->
            <div class="w-full h-[280px] flex flex-col mb-[10px] relative">
                <!-- 时间尺子 -->
                <div class="w-full min-h-[24px] mb-2 relative ml-[210px]">

                    <canvas ref="rulerRef" class="w-full h-[24px]" />
                    <!-- 播放头 -->
                    <Playhead class="absolute z-40" :timeMs="props.curTime" :pxPerSec="pxPerSec"
                        @startSeek="(time) => { emit('start-seek', time); }" @onSeek="(time) => emit('on-seek', time)"
                        @endSeek="(time) => emit('end-seek', time)" />
                </div>

                <!-- 时间轴视图 -->
                <div class="w-full h-[200px] flex-col flex justify-center space-y-1 overflow-auto">
                    <!-- 轨道 -->
                    <div 
                        v-for="(track, index) in VideoStudio.data.clipEngine?.project?.tracks" :key="index"
                        class="w-full h-[52px] bg-[#F5F5FA] relative flex flex-row"
                    >
                        <!-- 轨道头 -->
                        <div class="w-[200px] h-full bg-gray-300 mr-[10px] rounded-md flex-row-center-center space-x-2">
                            <span 
                                class="material-symbols-outlined"
                                @click="track.isLocked = !track.isLocked"
                            >
                                {{ track.isLocked? 'lock' : 'lock_open_right'}}
                            </span>
                            <span 
                                class="material-symbols-outlined"
                                @click="track.isVisible = !track.isVisible"
                            >
                                {{ track.isVisible? 'visibility' : 'visibility_off'}}
                            </span>
                            <span 
                                class="material-symbols-outlined"
                                @click="track.isMuted = !track.isMuted"
                            >
                                {{ track.isMuted? 'volume_off' : 'volume_up'}}
                            </span>
                        </div>
                        <!-- 绘制 clip 片段 -->
                        <div v-for="(clip, index) in track.clips"
                            class="h-full bg-black rounded-[10px] text-white flex flex-row" :style="{ 
                                left: `${clip.startTime / 1000 * pxPerSec}px`, 
                                width: `${clip.duration / 1000 * pxPerSec}px`, 
                                willChange: 'left' 
                            }">

                            <div v-if="clip.isEditing"
                                class="flex-row-between-center w-full h-full overflow-hidden items rounded-[10px] border border-purple">
                                <!-- 左把手 -->
                                <div class="w-[14px] h-full bg-purple flex-row-center-center">
                                    <div class="w-1/5 h-1/2 bg-white" />
                                </div>
                                <!-- 右把手 -->
                                <div class="w-[14px] h-full bg-purple flex-row-center-center">
                                    <div class="w-1/5 h-1/2 bg-white" />
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, ref, computed, watch, onMounted } from 'vue';
import { Utils } from 'clip-engine';

import Playhead from './Playhead.vue';

import { useTimeline } from './index';

import { VideoStudio } from '@frontend/store/videostudio';

defineOptions({ name: 'TimeLine' });
const props = defineProps({
    curTime: { type: Number, default: 0 },
    duration: { type: Number, default: 30000 },
    playing: { type: Boolean, default: false }
});
const emit = defineEmits(['update:playing', 'start-seek', 'on-seek', 'end-seek']);

const curTimeFormatted = computed(() => {
    return Utils.formatTime(props.curTime);
});
const { rulerRef, pxPerSec, drawRuler } = useTimeline(props.duration);

onMounted(() => {
    drawRuler();
});

</script>   