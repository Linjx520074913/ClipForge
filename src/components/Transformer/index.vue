<template>
    <div class='absolute' ref="rootRef" @mousedown.stop.prevent="startMove"
        :style="{ zIndex: zIndex }" >
        <!-- 内容插槽 -->
        <slot name="content"/>
        <!-- 四角控制点，插入到 body 中，这样超出预览区才可以显示 -->
        <Teleport :to="to">
            <div class='absolute ring-2 ring-purple pointer-events-none' v-if="selected"
                :style="anchorStyle">
                <div 
                    v-for="(p, index) in cornerAnchors" :key="index"
                    :class="['absolute w-[10px] h-[10px] bg-white border border-gray-400 rounded-full pointer-events-auto', anchorCls[p]]"
                    @mousedown.stop.prevent="startResize(p, $event)"
                />
                <span class="material-symbols-outlined absolute top-full mt-[10px] left-1/2">forward_media</span>
            </div>
        </Teleport>
    </div>
</template>

<script setup lang="ts">
/**
 * 功能描述：
    1、移动功能：
    组件通过 startMove 函数处理鼠标按下事件，支持拖拽移动组件。

    2、缩放功能：
    组件提供四个角的控制点，使用 startResize 函数处理缩放操作。

    3、外部点击监听：
    当点击组件外部时，触发 handleClickOutside 事件，取消选中状态。

    4、尺寸观察：
    通过 ResizeObserver 监听插槽内容的尺寸变化，动态调整组件的宽高。
 */
import { ref, defineOptions, defineProps, defineEmits, defineExpose, onMounted, onBeforeUnmount } from 'vue';

import { useResize, useMove, updateAnchorStyle } from './index';

defineOptions({ name: 'Transformer' });
const props = defineProps({
    width: { type: Number, default: 300 },
    height: { type: Number, default: 300 },
    zIndex: { type: Number, default: 1 },
    selected: { type: Boolean, default: false },
    to: { type: String, default: 'body' }
});
const emit = defineEmits<{
    (e: 'update:selected', value: boolean): void;
}>();

const rootRef = ref<HTMLElement | null>(null);

const anchorStyle = ref({ left: '0px', top: '0px', width: '0px', height: '0px', zIndex: 1});

let { cornerAnchors, anchorCls, startResize } = useResize(rootRef);

let { startMove } = useMove(props, emit, rootRef, anchorStyle)


/**
 * 父节点尺寸变化
 */
function onParentResize() {
    console.log('父组件尺寸变化通知：');
    updateAnchorStyle(rootRef, props, anchorStyle);
}


function handleClickOutside(event: MouseEvent) {
    console.error('FSDFSDFSDF handleClickCousdf', event.target,)
    emit('update:selected', false);
}

let observer: ResizeObserver;

onMounted(async() => {
    // 这一段的作用是设置 rootRef 的宽度和高度与 slot 里面的元素保持一致
    // 这样控制点和线框才能完整覆盖到 slot 上
    
    if (!rootRef.value) return;

    // 选出 slot 实际渲染的第一个元素
    const slotEl = rootRef.value.firstElementChild as HTMLElement;
    if (!slotEl) return;

    observer = new ResizeObserver((entries) => {
        for (const entry of entries) {
            const { width, height } = entry.contentRect;
            // 初始化元素尺寸
            // 如果不设置的话，在移动到预览区域的右边的时候，会挤压这个组件
            rootRef.value!.style.width = `${width}px`;
            rootRef.value!.style.height = `${height}px`;
            
            updateAnchorStyle(rootRef, props, anchorStyle);
        }
    });

    observer.observe(slotEl);
    document.addEventListener('mousedown', handleClickOutside);
});

onBeforeUnmount(() => {
    observer?.disconnect(); 
    document.removeEventListener('mousedown', handleClickOutside);
})

/**
 * 接口暴露  
 */ 
defineExpose({
    onParentResize
})

</script>

<style scoped>

</style>