<template>
    <div class="flex flex-col">
        <!-- 控制条 -->
        <div class="w-full bg-gray-100 h-[40px] flex items-center justify-center">
            <!-- 播放暂停按钮 -->
            <span @click="emit('update:playing', !playing)"
                class="material-icons text-[40px] cursor-pointer hover:scale-110 transition-all duration-250 hover:text-purple">
                {{ playing ? 'pause_circle' : 'play_circle' }}
            </span>
        </div>
        <!-- <TimelineRuler 
            :duration="10"
            class="w-full h-[80px] bg-green-200"
        />
        <TimelineViewport class="w-full flex-1 bg-blue-300"/> -->
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, ref } from 'vue';

defineOptions({ name: 'TimeLine' });
defineProps({
    playing: { type: Boolean, default: false }
});
const emit = defineEmits(['update:playing']);

</script>   