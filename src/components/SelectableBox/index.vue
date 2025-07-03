<template>
    <!-- 外层：盒子用于定位和尺寸控制 -->
    <div
      ref="boxRef"
      :style="boxStyle"
    >
      <!-- 内容插槽 -->
      <slot ref="contentRef" />

      <!-- 四角控制点，选中时显示 -->
      <Teleport to="body" v-if="selected">
        <div
          :class="['absolute z-50 pointer-events-none', selected ? 'ring-2 ring-purple' : '']"
          :style="teleportStyle"
        >
          <div
            v-for="(p, index) in pos"
            :key="index"
            :class="['absolute w-[10px] h-[10px] bg-white border border-gray-400 rounded-full', posCls[p]]"
            @mousedown.prevent="startResize(p, $event)"
            style="pointer-events: auto"
          />
        </div>
      </Teleport>
    </div>
  </template>
  
  <script setup lang="ts">
  import {
    defineOptions,
    defineProps,
    defineEmits,
    ref,
    reactive,
    onMounted,
    onBeforeUnmount,
    nextTick,
    watch
  } from 'vue'
  
  defineOptions({ name: 'SelectableBox' })
  
  const props = defineProps({
    selected: { type: Boolean, default: true },
  })
  
  const emit = defineEmits<{
    (e: 'resize', payload: { width: number; height: number }): void
    (e: 'update:selected', value: boolean): void
  }>()
  
  const boxRef = ref<HTMLElement | null>(null)
  const contentRef = ref<HTMLElement | null>(null)
  let ratio = 1
  
  const pos = ['tl', 'tr', 'bl', 'br'] as const
  const posCls: Record<string, string> = {
    tl: 'top-[-5px]    left-[-5px]  cursor-nwse-resize',
    tr: 'top-[-5px]    right-[-6px] cursor-nesw-resize',
    bl: 'bottom-[-5px] left-[-5px]  cursor-nesw-resize',
    br: 'bottom-[-5px] right-[-5px] cursor-nwse-resize',
  }
  
  const teleportStyle = ref({ top: '0px', left: '0px', width: '0px', height: '0px' })
  
  let start = { left: 0, top: 0, x: 0, y: 0, w: 0, h: 0 }
  let curHandle: typeof pos[number] | null = null
  
  const boxStyle = reactive({ top: '0px', left: '0px' })
  
  function startResize(handle: typeof pos[number], e: MouseEvent) {
    if (!boxRef.value) return
  
    curHandle = handle
    start = {
      left: boxRef.value.offsetLeft,
      top: boxRef.value.offsetTop,
      x: e.clientX,
      y: e.clientY,
      w: boxRef.value.offsetWidth,
      h: boxRef.value.offsetHeight,
    }
    ratio = start.w / start.h
  
    window.addEventListener('mousemove', onResize)
    window.addEventListener('mouseup', endResize)
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
  
    updateTeleport()
    emit('resize', { width: newW, height: newH })
  }
  
  function endResize() {
    window.removeEventListener('mousemove', onResize)
    window.removeEventListener('mouseup', endResize)
    curHandle = null
  }
  
  function updateTeleport() {
    if (!boxRef.value) return
    const rect = boxRef.value.getBoundingClientRect()
    teleportStyle.value = {
      top: `${rect.top + window.scrollY}px`,
      left: `${rect.left + window.scrollX}px`,
      width: `${rect.width}px`,
      height: `${rect.height}px`,
    }
  }
  
  watch(() => props.selected, (val) => {
    if (val) nextTick(updateTeleport)
  })
  
  onMounted(() => {
    updateTeleport()
    window.addEventListener('resize', updateTeleport)
  })
  
  onBeforeUnmount(() => {
    window.removeEventListener('resize', updateTeleport)
  })
  </script>
  
  <style scoped>
  </style>
  