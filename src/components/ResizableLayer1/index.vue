<template>
    <div class='relative inline-block' ref="rootRef1" >
        <!-- 内容插槽 -->
        <slot name="content"/>
        <!-- 四角控制点，插入到 body 中，这样超出预览区才可以显示 -->
        <Teleport :to="to">
            <div class='absolute ring-2 ring-purple pointer-events-auto' v-if="selected" @mousedown.stop.prevent="mouseDown"
                ref="controlRef"
                :style="style">
                <div 
                    v-for="(p, index) in pos" :key="index"
                    :class="['absolute w-[10px] h-[10px] bg-white border border-gray-400 rounded-full', posCls[p]]"
                    @mousedown.stop.prevent="startResize(p, $event)"
                />
                <span class="material-symbols-outlined absolute top-full mt-[10px] left-1/2">forward_media</span>
            </div>
        </Teleport>
    </div>
</template>

<script setup lang="ts">
import { ref, defineOptions, defineProps, defineEmits, defineExpose, computed, onMounted, onBeforeUnmount } from 'vue';
defineOptions({ name: 'ResizableLayer1' });
const props = defineProps({
    width: { type: Number, default: 300 },
    height: { type: Number, default: 300 },
    selected: { type: Boolean, default: true },
    to: { type: String, default: 'body' }
});
const emit = defineEmits<{
    (e: 'update:selected', value: boolean): void;
}>();

const rootRef1 = ref<HTMLCanvasElement | null>(null);
const controlRef = ref<HTMLDivElement | null>(null);

const style = ref({
    left: '0',
    top: '0',
    width: '100%',
    height: '100%'
});

function updateStyle() {
    const rect = rootRef1.value?.getBoundingClientRect();
    if (!rect){
        return
    }

    style.value = {
        left: `${rect.left}px`,
        top: `${rect.top}px`,
        width: `${rect.width}px`,
        height: `${rect.height}px`
    }
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
    if(!rootRef1.value){
        return;
    }

    curHandle = handle;
    start = {
      left: rootRef1.value.offsetLeft,
      top: rootRef1.value.offsetTop,
      x: e.clientX,
      y: e.clientY,
      w: rootRef1.value.offsetWidth,
      h: rootRef1.value.offsetHeight,
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
            rootRef1.value.style.left = start.left + dx + 'px';
            rootRef1.value.style.top = start.top + dy + 'px';
            break;
        case 'tr':
            // 拖动右上角, left 不变，改变 top
            newW = newW + dx;
            rootRef1.value.style.top = start.top - dy + 'px';
            break;
        case 'bl':
            // 拖动左下角，top 不变，改变 left
            newW = newW - dx;
            rootRef1.value.style.left = start.left + dx + 'px';
            break;
        case 'br':
            // 拖动右下角，left 和 top 都不变
            newW = newW + dx;
            break;
        default:
            break;
    }
    newH = newW / ratio;

    rootRef1.value.style.width = `${newW}px`
    rootRef1.value.style.height = `${newH}px`;
  
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
        left: rootRef1.value.offsetLeft,
        top: rootRef1.value.offsetTop,
        x: e.clientX,
        y: e.clientY,
        w: rootRef1.value.offsetWidth,
        h: rootRef1.value.offsetHeight,
    };
    window.addEventListener('mousemove', mouseMove);
    window.addEventListener('mouseup', mouseUp);
}

function mouseMove(e: MouseEvent) {
    const dx = e.clientX - start.x;
    const dy = e.clientY - start.y;

    rootRef1.value.style.left = `${start.left + dx}px`;
    rootRef1.value.style.top = `${start.top + dy}px`;

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
  const insideRoot = isInRect(event, rootRef1.value);
  const insideControl = isInRect(event, controlRef.value);

  if (!insideRoot && !insideControl) {
    emit('update:selected', false);
  }
}


onMounted(() => {
    document.addEventListener('click', handleClickOutside);
})

onBeforeUnmount(() => {
    document.removeEventListener('click', handleClickOutside);
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