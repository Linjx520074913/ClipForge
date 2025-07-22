<template>
    <div class="flex flex-col">
        <!-- 控制条 -->
        <div class="w-full bg-gray-100 h-[40px] flex items-center justify-center">
            <!-- 播放暂停按钮 -->
            <span @click="emit('update:playing', !playing)"
                class="material-icons text-[40px] cursor-pointer hover:scale-110 transition-all duration-250 hover:text-purple">
                {{ playing ? 'pause_circle' : 'play_circle' }}
            </span>
            <div class="text-[12px]">{{ curTimeFormatted }} / 0:00:00</div>
        </div>
        <!-- 时间轴视图 -->
        <div class="w-full h-full relative flex p-[10px] test">
            <!-- 时间尺子 -->
            <div class="w-full bg-green-200 h-[24px] absolute z-20"></div>
            <!-- 播放头 -->
            <Playhead 
                class="absolute z-30" :timeMs="props.curTime" 
                @startSeek="(time) => { emit('start-seek', time); }"
                @onSeek="(time) => emit('on-seek', time)"
                @endSeek="(time) => emit('end-seek', time)"/>
            <!-- 轨道 -->
            <div class="w-full h-full z-31">
                轨道
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, ref, computed, watch } from 'vue';
import { Utils } from 'clip-engine';

import Playhead from './Playhead.vue';

defineOptions({ name: 'TimeLine' });
const props = defineProps({
    curTime: { type: Number, default: 0 },
    duration: { type: Number, default: 0 },
    playing: { type: Boolean, default: false }
});
const emit = defineEmits(['update:playing', 'start-seek', 'on-seek', 'end-seek']);

const curTimeFormatted = computed(() => {
    return Utils.formatTime(props.curTime);
});

</script>   