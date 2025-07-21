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
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, ref, computed, watch } from 'vue';

defineOptions({ name: 'TimeLine' });
const props = defineProps({
    curTime: { type: Number, default: 0 },
    duration: { type: Number, default: 0 },
    playing: { type: Boolean, default: false }
});
const emit = defineEmits(['update:playing']);

function formatTime(durationMs: number): string {
    const hours = Math.floor(durationMs / 3600000);
    const minutes = Math.floor((durationMs % 3600000) / 60000);
    const seconds = Math.floor((durationMs % 60000) / 1000);
    const milliseconds = durationMs % 1000;

    const pad = (num: number, size: number) => String(num).padStart(size, '0');

    return `${pad(hours, 2)}:${pad(minutes, 2)}:${pad(seconds, 2)}:${pad(milliseconds, 3)}`;
}

const curTimeFormatted = computed(() => {
    return formatTime(props.curTime);
});

watch(() => props.curTime, (newVal, oldVal) => {
    console.log(`curTime changed from ${oldVal} to ${newVal}`);
}, { immediate: true });

</script>   