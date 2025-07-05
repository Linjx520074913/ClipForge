<template>
    <div class='absolute' ref="rootRef" @mousedown.stop.prevent="mouseDown"
        :style="{ zIndex: zIndex }" >
        <!-- 内容插槽 -->
        <slot name="content"/>
        <!-- 四角控制点，插入到 body 中，这样超出预览区才可以显示 -->
        <Teleport :to="to">
            <div class='absolute ring-2 ring-purple pointer-events-none' v-if="selected"
                ref="controlRef"
                :style="style">
                <div 
                    v-for="(p, index) in pos" :key="index"
                    :class="['absolute w-[10px] h-[10px] bg-white border border-gray-400 rounded-full pointer-events-auto', posCls[p]]"
                    @mousedown.stop.prevent="startResize(p, $event)"
                />
                <span class="material-symbols-outlined absolute top-full mt-[10px] left-1/2">forward_media</span>
            </div>
        </Teleport>
    </div>
</template>

<script setup lang="ts">
import { ref, defineOptions, defineProps, defineEmits, defineExpose, onMounted, onBeforeUnmount } from 'vue';
defineOptions({ name: 'ResizableBox' });
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

const rootRef = ref<HTMLCanvasElement | null>(null);
const controlRef = ref<HTMLDivElement | null>(null);

const style = ref({
    left: '0',
    top: '0',
    width: '0px',
    height: '0px'
});

function updateStyle() {
    if (!rootRef.value) return;

    // 获取 rootRef 的位置
    const rect = rootRef.value.getBoundingClientRect();
    
    // 获取 Teleport 目标容器的位置
    const teleportTarget = document.querySelector(props.to) as HTMLElement;
    const teleportRect = teleportTarget?.getBoundingClientRect() || { left: 0, top: 0 };

    // 更新控制框的样式
    style.value = {
        left: `${rect.left - teleportRect.left}px`,
        top: `${rect.top - teleportRect.top}px`,
        width: `${rect.width}px`,
        height: `${rect.height}px`,
        zIndex: props.zIndex
    };
    
}

// 控制点
const pos = ['tl', 'tr', 'bl', 'br'];
// 控制点的类
const posCls: Record<string, string> = {
    tl: 'top-0 left-0 -translate-x-1/2 -translate-y-1/2 cursor-nwse-resize',
    tr: 'top-0 right-0 translate-x-1/2 -translate-y-1/2 cursor-nesw-resize',
    bl: 'bottom-0 left-0 -translate-x-1/2 translate-y-1/2 cursor-nesw-resize',
    br: 'bottom-0 right-0 translate-x-1/2 translate-y-1/2 cursor-nwse-resize',
};

/**
 * 父节点尺寸变化
 */
function onParentResize() {
    console.log('父组件尺寸变化通知：');
    updateStyle();
}

let start = { left: 0, top: 0, x: 0, y: 0, w: 0, h: 0 }
let curHandle: typeof pos[number] | null = null;
let ratio = 1

function startResize(handle: typeof pos[number], e: MouseEvent) {
    if(!rootRef.value){
        return;
    }

    curHandle = handle;
    start = {
      left: rootRef.value.offsetLeft,
      top: rootRef.value.offsetTop,
      x: e.clientX,
      y: e.clientY,
      w: rootRef.value.offsetWidth,
      h: rootRef.value.offsetHeight,
    };
    ratio = start.w / start.h;
  
    window.addEventListener('mousemove', onResize);
    window.addEventListener('mouseup', endResize);
}
  
function onResize(e: MouseEvent) {
    if(!curHandle) return;

    const dx = e.clientX - start.x;
    const dy = dx / ratio;

    let newW = start.w;
    let newH = start.h;

    switch(curHandle){
        case 'tl':
            // 拖动左上角，改变 left，top
            newW = newW - dx;
            rootRef.value.style.left = start.left + dx + 'px';
            rootRef.value.style.top = start.top + dy + 'px';
            break;
        case 'tr':
            // 拖动右上角, left 不变，改变 top
            newW = newW + dx;
            rootRef.value.style.top = start.top - dy + 'px';
            break;
        case 'bl':
            // 拖动左下角，top 不变，改变 left
            newW = newW - dx;
            rootRef.value.style.left = start.left + dx + 'px';
            break;
        case 'br':
            // 拖动右下角，left 和 top 都不变
            newW = newW + dx;
            break;
        default:
            break;
    }
    newH = newW / ratio;

    rootRef.value.style.width = `${newW}px`
    rootRef.value.style.height = `${newH}px`;
  
    updateStyle();
    // emit('resize', { width: newW, height: newH })
}
  
function endResize() {
    window.removeEventListener('mousemove', onResize)
    window.removeEventListener('mouseup', endResize)
    curHandle = null
}

function mouseDown(e: MouseEvent){
    emit('update:selected', true);
    start = {
        left: rootRef.value.offsetLeft,
        top: rootRef.value.offsetTop,
        x: e.clientX,
        y: e.clientY,
        w: rootRef.value.offsetWidth,
        h: rootRef.value.offsetHeight,
    };
    window.addEventListener('mousemove', mouseMove);
    window.addEventListener('mouseup', mouseUp);
}

function mouseMove(e: MouseEvent) {
    const dx = e.clientX - start.x;
    const dy = e.clientY - start.y;

    rootRef.value.style.left = `${start.left + dx}px`;
    rootRef.value.style.top = `${start.top + dy}px`;

    updateStyle();
}

function mouseUp(e: MouseEvent){
    window.removeEventListener('mousemove', mouseMove);
    window.removeEventListener('mouseup', mouseUp);
}

function isInRect(e: MouseEvent, el: HTMLElement | null) {
        if (!el) return false;
        const rect = el.getBoundingClientRect();
        return (
            e.clientX >= rect.left &&
            e.clientX <= rect.right &&
            e.clientY >= rect.top &&
            e.clientY <= rect.bottom
        );
    }

function handleClickOutside(event: MouseEvent) {
    // 用鼠标坐标的方法，判断是否点击到了组件外
    // TODO: 如果组件发生重叠，这种方法就是不能判断是否点击到了组件外
    
    const insideRoot = isInRect(event, rootRef.value);
    const insideControl = isInRect(event, controlRef.value);

    if (!insideControl) {
        emit('update:selected', false);
    }
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
            updateStyle(); // 同步控制框
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