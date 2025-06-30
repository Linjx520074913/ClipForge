<template>
    <div class="flex flex-row">
        <div class="flex flex-col flex-1 overflow-hidden min-w-[300px] rounded-[15px] shadow-sm border bg-white">
            <!-- 主渲染区域 -->
            <div class="stage-canvas w-full h-[540px] relative border-b border-b-black grid place-items-center" ref="stageCanvasRef">
                <!-- 预览区域 -->
                <div class="canvas-container border border-red-800 relative" ref="canvasContainerRef">
                    <!-- 顶部工具条 -->
                    <VideoStudioToolbar 
                        class="absolute w-[158px] h-[34px] rounded-[5px] shadow-[0_3px_10px_rgba(0,0,0,0.12)] bottom-full mb-2 left-1/2 -translate-x-1/2"
                        @update:ratio="handleRatioUpdate"/>
                    <MediaPlayer/>
                </div>
            </div>
            <!-- 时间轴 -->
        </div>
        <!-- 分割线 -->
        <div class="resize w-[7px] h-full"></div>
        <!-- 右侧面板 -->
        <SlidingPanel class="h-full" :items="rightSlidingItems" :expanded="false"/>
    </div>
    
</template>

<script setup lang="ts">
import {
    SlidingPanel,
    VideoStudioToolbar
} from '@src/components/index'
import { AspecRatioItem } from '@src/components/VideoStudioToolbar';
import { PanelItem } from '@src/components/SlidingPanel';

import { 
    AudioPanel,
    ColorPanel,
    EffectPanel,
    FilterPanel
} from '@src/page/SliderBarRight/index';

import { defineOptions, ref, onMounted, onBeforeUnmount } from 'vue';
defineOptions({ name: 'VideoStudio' });

const rightSlidingItems: PanelItem[] = [
    { id: 0, label: '音频', icon: 'volume_up',      tooltip: 'test', component: AudioPanel },
    { id: 1, label: '滤镜', icon: 'filter_vintage', tooltip: 'test', component: ColorPanel },
    { id: 2, label: '效果', icon: 'contrast',       tooltip: 'test', component: EffectPanel },
    { id: 3, label: '颜色', icon: 'palette',        tooltip: 'test', component: FilterPanel } ,
]

const stageCanvasRef = ref<HTMLElement | null>();
const canvasContainerRef = ref<HTMLElement | null>();
let resizeObserver: ResizeObserver | null = null

let ratio = 16 / 9;

/**
 * 处理视频比例更新
 * @param item 
 */
function handleRatioUpdate(item: AspecRatioItem){
    if(!item) return;

    const [w, h] = item.value?.split(':').map(Number)!;
    ratio = w / h;
    resizeCanvasContainer(ratio);
}

/**
 * 重置容器尺寸
 * @param ratio  目标比例, 比如 16/9
 */
function resizeCanvasContainer(ratio: number){

    // padding
    const padding = { h: 200, v: 100 };
    // 渲染区域尺寸
    const stageCanvas = stageCanvasRef.value!;
    const w = stageCanvas.clientWidth - padding.h;
    const h = stageCanvas.clientHeight - padding.v;

    let targetW, targetH;

    // 先按宽度算高度
    targetW = w;
    targetH = w / ratio;

    // 如果高度超出，就用高度算宽度
    if(targetH > h){
        targetH = h;
        targetW = h * ratio;
    }

    canvasContainerRef.value!.style.width = `${targetW}px`
    canvasContainerRef.value!.style.height = `${targetH}px`
}

onMounted(() => {
    if (stageCanvasRef.value) {
        resizeObserver = new ResizeObserver(entries => {
            for (const entry of entries) {
                const { width, height } = entry.contentRect
                // containerSize.value = { width, height }
                console.log('📏 canvas 尺寸变了：', width, height)
                resizeCanvasContainer(ratio)
            }
        })

        resizeObserver.observe(stageCanvasRef.value)
    }
})

onBeforeUnmount(() => {
    if (resizeObserver && canvasContainerRef.value) {
        resizeObserver.unobserve(canvasContainerRef.value)
        resizeObserver.disconnect()
    }
})
</script>

<style scoped>

</style>