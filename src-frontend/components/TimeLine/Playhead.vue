<template>
    <!-- 整个播放指针区域 -->
    <div 
        ref="rootRef"
        class="relative h-[90%] w-[3px] bg-black rounded-xl translate-y-[20px] shadow-[0_2px_6px_rgba(0,0,0,0.5)] cursor-grab active:cursor-grabbing"
        :style="{ left: pos, willChange: 'left' }"
        @mousedown="onMouseDown"
    >
        <!-- 指针头部 SVG（尖尖） -->
        <svg
            class="absolute top-0 left-1/2 -translate-x-1/2 -translate-y-[90%] transition-all duration-200 hover:scale-110"
            width="18"
            height="20"
            viewBox="0 0 18 20"
            fill="var(--colorNeutralForeground1)"
        >
            <path d="M0 6C0 2.68629 2.68629 0 6 0H12C15.3137 0 18 2.68629 18 6V9.95748C18 11.5005 17.4055 12.9842 16.3401 14.1003L11.8934 18.7588C10.3177 20.4096 7.68234 20.4096 6.10658 18.7588L1.65987 14.1003C0.594455 12.9842 0 11.5005 0 9.95748V6Z"/>
        </svg>
    </div>
</template>

<script setup lang="ts">
import { computed, defineOptions } from 'vue';
import { useMouse } from './Playhead';

defineOptions({ name: 'Playhead' });
const props = defineProps({
    timeMs: { type: Number, default: 0 }
});
const emit = defineEmits(['update:time-ms', 'start-seek', 'end-seek', 'on-seek']);

const { actived, rootRef, pos, onMouseDown } = useMouse(props, emit);


</script>