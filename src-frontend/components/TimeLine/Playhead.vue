<template>
    <!-- 整个播放指针区域 -->
    <div 
        ref="rootRef"
        class="h-[200px] w-[5px] bg-black border border-white rounded-xl translate-y-[10px] cursor-grab active:cursor-grabbing"
        :style="{ left: pos, top: '10px', willChange: 'left' }"
        @mousedown="onMouseDown"
    >
        <transition name="fade-slide">
            <span
                v-if="actived"
                class="absolute flex-row-center-center w-[60px] h-[20px] 
                    top-0 left-1/2 -translate-x-1/2 -translate-y-[200%] 
                    bg-black text-white text-[12px] rounded-md"
            >
                {{ formattedTime }}
            </span>
        </transition>
        <!-- 指针头部 SVG（尖尖） -->
        <svg
            class="absolute top-0 left-1/2 -translate-x-1/2 -translate-y-[90%] transition-all duration-200 hover:scale-110"
            width="18"
            height="20"
            viewBox="0 0 18 20"
            :fill="actived ? '#892fff' : ''"
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
    timeMs: { type: Number, default: 0 },
    pxPerSec: { type: Number, default: 30 }
});

const emit = defineEmits(['update:time-ms', 'start-seek', 'end-seek', 'on-seek']);

function formatTime(ms: number): string {
    const totalSeconds = Math.floor(ms / 1000);
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = totalSeconds % 60;
    const milli = Math.floor((ms % 1000) / 10)
                   .toString()
                   .padStart(2, "0");

    const m = minutes.toString();
    const s = seconds.toString().padStart(2, "0");

    return `${m}:${s}:${milli}`;
}

const formattedTime = computed(() => formatTime(props.timeMs));

const { actived, rootRef, pos, onMouseDown } = useMouse(props, emit);


</script>

<style scoped>
.fade-slide-enter-active,
.fade-slide-leave-active {
  transition: all 0.3s cubic-bezier(0.3, 1.4, 0.5, 1); /* 弹性 */
}

/* 👇 初始状态：透明 + 位置低 + 缩小 */
.fade-slide-enter-from {
  opacity: 0;
  transform: translate(-50%, -180%) scale(0.8);
}

/* 👇 进入后：不透明 + 上移 + 放大到正常 */
.fade-slide-enter-to {
  opacity: 1;
  transform: translate(-50%, -200%) scale(1);
}

/* 👇 离开时：透明 + 缩小 + 下移 */
.fade-slide-leave-from {
  opacity: 1;
  transform: translate(-50%, -200%) scale(1);
}
.fade-slide-leave-to {
  opacity: 0;
  transform: translate(-50%, -180%) scale(0.8);
}

</style>