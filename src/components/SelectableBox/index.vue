<template>
    <div :class="['relative', selected? 'ring-2 ring-purple' : '']"
        ref="boxRef">
        <!-- 内容插槽 -->
        <slot ref="contentRef"/>
        <!-- 四角控制点，选中时显示 -->
        <template v-if="selected">
            <div 
                v-for="(p, index) in pos" :key="index"
                :class="['absolute w-[10px] h-[10px] bg-white border border-gray-400 rounded-full', posCls[p]]"
                @mousedown.prevent='startResize(p, $event)'
                />
        </template>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, defineEmits, ref, onMounted, onBeforeUnmount } from 'vue';
defineOptions({ name: 'SelectableBox'});

defineProps({
    selected: { type: Boolean, default: false }
});

const emit = defineEmits<{
    (e: 'resize', payload: { width: number, height: number}): void;
    (e: 'update:selected', value: boolean): void;
}>();

const boxRef = ref<HTMLElement | null>(null);
const contentRef = ref<HTMLElement | null>(null);
let ratio = 1;

// 控制点
const pos = ['tl', 'tr', 'bl', 'br'];
// 控制点的类
const posCls: Record<string, string> = {
    tl: 'top-[-5px]    left-[-5px]  cursor-nwse-resize',
    tr: 'top-[-5px]    right-[-6px] cursor-nesw-resize',
    bl: 'bottom-[-5px] left-[-5px]  cursor-nesw-resize',
    br: 'bottom-[-5px] right-[-5px] cursor-nwse-resize'
};

let start = { left: 0, top: 0, x: 0, y: 0, w: 0, h: 0};
let curHandle: typeof pos[number] | null = null;

/**
 * 开始缩放
 * @param handle 控制点
 * @param e 事件
 */
function startResize(handle: typeof pos[number], e: MouseEvent){
    console.error('startResize', boxRef.value.offsetLeft);
    e.preventDefault();
    curHandle = handle;
    start = {
        left: boxRef.value.offsetLeft,
        top: boxRef.value.offsetTop,
        x: e.clientX,
        y: e.clientY,
        w: boxRef.value.offsetWidth,
        h: boxRef.value.offsetHeight
    };
    
    ratio = boxRef.value.offsetWidth / boxRef.value.offsetHeight;

    window.addEventListener('mousemove', onResize);
    window.addEventListener('mouseup', endResize);
}

/**
 * 拖放 4 个角控制点，缩放
 */
function onResize(e: MouseEvent){
    if(!curHandle) return;

    const dx = e.clientX - start.x;
    const dy = dx / ratio;

    let newW = start.w;
    let newH = start.h;

    switch(curHandle){
        case 'tl':
            // 拖动左上角，改变 left，top
            newW = newW - dx;
            boxRef.value.style.left = start.left + dx + 'px';
            boxRef.value.style.top = start.top + dy + 'px';
            break;
        case 'tr':
            // 拖动右上角, left 不变，改变 top
            newW = newW + dx;
            boxRef.value.style.top = start.top - dy + 'px';
            break;
        case 'bl':
            // 拖动左下角，top 不变，改变 left
            newW = newW - dx;
            boxRef.value.style.left = start.left + dx + 'px';
            break;
        case 'br':
            // 拖动右下角，left 和 top 都不变
            newW = newW + dx;
            break;
        default:
            break;
    }
    newH = newW / ratio;

    boxRef.value.style.width = `${newW}px`
    boxRef.value.style.height = `${newH}px`;

    emit('resize', { width: newW, height: newH });

}

/**
 * 结束缩放
 */

function endResize(){
    window.removeEventListener('mousemove', onResize);
    window.removeEventListener('mouseup', endResize);
    curHandle = null;
}

const handleClickOutside = (event: MouseEvent) => {
    const target = event.target as Node;
    if(boxRef.value && !boxRef.value.contains(target)){
        emit('update:selected', false);
    }
}

onMounted(() => {
    document.addEventListener('click', handleClickOutside);
})

onBeforeUnmount(() => {
    document.removeEventListener('click', handleClickOutside);
})

</script>

<style scoped>

</style>
