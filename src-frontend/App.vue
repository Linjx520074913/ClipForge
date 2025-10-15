<template>
    <div 
        ref="appRoot"
        class="flex flex-col h-screen select-none box-border relative"
    >
        <div 
            class="absolute w-full h-full bg-white z-10"
            :style="style"
        />
        <AppHeader 
            class="w-full h-[56px] z-20"
            style="padding: 8px 12px 8px 16px"
        />
        <Studio 
            class="w-full flex-1 h-0 mb-[5px] z-20"
        />
    </div>
</template>

<script setup lang="ts">
/**
 * TODO
 * 1、所有交互添加提示，比如 按钮
 */
import { AppHeader } from '@frontend/components';
import { TestClipEngine } from '@frontend/test';
import { Studio } from '@frontend/pages';
import { computed } from 'vue';
import { State } from '@frontend/store/state';

const appRoot = ref<HTMLElement | null>(null);
const style = computed(() => {
   const w = appRoot.value?.clientWidth || 800;
   const h = appRoot.value?.clientHeight || 600;
   const scale = window.devicePixelRatio;
   return `
        -webkit-mask: 
            linear-gradient(black, black) content-box, 
            linear-gradient(black, black);
            -webkit-mask-composite: xor;

            padding:${State.data.render_wnd_pos.y / scale}px 
                    ${w - State.data.render_wnd_pos.x / scale - State.data.render_wnd_size.w / scale}px
                    ${h - State.data.render_wnd_pos.y / scale - State.data.render_wnd_size.h  / scale}px
                    ${State.data.render_wnd_pos.x / scale}px;
   `;
});
</script>

<style>
.flex-x-center{
    @apply flex flex-row justify-center items-center;
}
.flex-col-center-center {
    @apply flex flex-col justify-center items-center;
}
.flex-row-center-center{
    @apply flex flex-row justify-center items-center;
}
.flex-row-end-center{
    @apply flex flex-row justify-end items-center;
}
.flex-row-between-center{
    @apply flex flex-row justify-between items-center;
}
</style>