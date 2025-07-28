<template>
    <div 
        class="flex flex-col"
        id="timeline"
    >
        <!-- 控制条 -->
        <div class="w-full h-[40px] flex items-center justify-center">
            <!-- 播放暂停按钮 -->
            <span @click="emit('update:playing', !playing)"
                class="material-icons text-[40px] cursor-pointer hover:scale-110 transition-all duration-250 hover:text-purple">
                {{ playing ? 'pause_circle' : 'play_circle' }}
            </span>
            <div class="text-[12px]">{{ curTimeFormatted }} / 0:00:00</div>
        </div>
        <div class="w-full h-[280px] px-[10px] flex flex-col mb-[10px] relative">
            <div class="w-full min-h-[24px] mb-2 relative">
                <!-- 时间尺子 -->
                <canvas ref="rulerRef" class="w-full h-[24px]"/>
                <!-- 播放头 -->
                <Playhead 
                    class="absolute z-40" 
                    :timeMs="props.curTime"
                    :pxPerSec="pxPerSec" 
                    @startSeek="(time) => { emit('start-seek', time); }"
                    @onSeek="(time) => emit('on-seek', time)"
                    @endSeek="(time) => emit('end-seek', time)"/>
            </div>
            
            <!-- 时间轴视图 -->
            <div class="w-full h-[200px] flex-col flex justify-center space-y-1 overflow-y-scroll py-[10px]">
                <!-- 轨道 -->
                <div
                    v-for="(track, index) in VideoStudio.data.clipEngine?.project?.tracks" :key="index"
                    class="w-full h-[52px] bg-[#F5F5FA] relative"
                >
                    <!-- 绘制 clip 片段 -->
                    <div
                        v-for="(clip, index) in track.clips"
                        class="h-full bg-white rounded-xl text-white relative p-1"
                        :style="{ 
                            left: `${clip.startTime / 1000 * pxPerSec}px`, 
                            width: `${clip.duration / 1000 * pxPerSec}px`, 
                            willChange: 'left' 
                        }"
                    >
                        <!-- <div
                            :class="[clip.isEditing? 'absolute w-full h-full top-0 left-0 border-2 border-purple ring-purple rounded-xl p-1': '']"
                        >
                            <div 
                                class="w-full h-full border-2 border-white rounded-md">
                            aaa
                            </div>
                        </div> -->
                        <div 
                            v-if="clip.isEditing"
                            class="w-full h-full rounded-md ring-1 ring-purple-500 ring-offset-1 ring-offset-purple transition-all duration-200">
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