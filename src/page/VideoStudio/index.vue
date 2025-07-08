<template>
    <div class="flex flex-row w-full">
        <div class="flex flex-col flex-1 overflow-hidden overflow-x-scroll min-w-[300px] rounded-[15px] shadow-sm border bg-white">
            <!-- 主渲染区域 -->
            <div class="stage-canvas w-full h-[778px] relative border-b border-b-gray-300 grid place-items-center" ref="stageCanvasRef">
                <!-- 顶部工具条 -->
                <VideoStudioToolbar 
                        class="absolute w-[158px] h-[34px] rounded-[5px] shadow-[0_3px_10px_rgba(0,0,0,0.12)] top-2 left-1/2 -translate-x-1/2"
                        @update:ratio="handleRatioUpdate"/>
                <!-- 预览区域,根据比例重置过大小 -->
                <div class="canvas-container border border-red-800 relative overflow-hidden" ref="canvasContainerRef">
                    <!-- 生成 layer 层, layer 中包含 Video/Audio/Text/Image 等 -->
                    <Transformer v-for="(layer, index) in layers" :key="index" 
                        v-if="canvasContainerRef"
                        v-model:selected="layer.active"
                        :zIndex="layer.zIndex"
                        :to="'.stage-canvas'"
                        >
                        <template #content>
                            <video v-if="layer.type == 'video'" :src="layer.source.uri" @mousedown="layer.active = true"/>
                            <img class="object-contain w-full h-full" 
                                v-if="layer.type == 'image'" 
                                :src="layer.source.uri" 
                                @mousedown="layer.active = true"
                                draggable="false"/>
                            <!-- TODO: 文字的拉伸有问题 -->
                            <div v-if="layer.type == 'text'" @mousedown="layer.active = true">{{ layer.source.text }} </div>
                        </template>
                    </Transformer>
                </div>
            </div>
            <!-- 时间轴 -->
            <!-- <TimeLine class="min-w-full" /> -->
        </div>
        <!-- 分割线 -->
        <div class="resize w-[7px] h-full"></div>
        <!-- 右侧面板 -->
        <SlidingPanel class="h-full" :items="rightSlidingItems" :expanded="true"/>
    </div>
    
</template>

<script setup lang="ts">
import {
    SlidingPanel,
    VideoStudioToolbar,
    MediaPlayer,
    Transformer,
    TimeLine
} from '@src/components/index'
import { AspecRatioItem } from '@src/components/VideoStudioToolbar';
import { PanelItem } from '@src/components/SlidingPanel';

import { useVideoStudio } from './index';

import { 
    AudioPanel,
    ColorPanel,
    EffectPanel,
    FilterPanel
} from '@src/page/SliderBarRight/index';

import { defineOptions, ref, onMounted, onBeforeUnmount } from 'vue';
defineOptions({ name: 'VideoStudio' });

const { layers } = useVideoStudio();

// 右侧侧边栏菜单
const rightSlidingItems: PanelItem[] = [
    { id: 0, label: '音频', icon: 'volume_up',      tooltip: 'test', component: AudioPanel },
    { id: 1, label: '滤镜', icon: 'filter_vintage', tooltip: 'test', component: FilterPanel },
    { id: 2, label: '效果', icon: 'contrast',       tooltip: 'test', component: EffectPanel },
    { id: 3, label: '颜色', icon: 'palette',        tooltip: 'test', component: ColorPanel } ,
];

const mediaRef = ref(null);

const playerSize = ref({ w: 0, h: 0 });

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

let targetW = 0, targetH = 0;
/**
 * 重置容器尺寸
 * @param ratio  目标比例, 比如 16/9
 */
function resizeCanvasContainer(ratio: number){

    // padding
    const padding = { h: 100, v: 100 };
    // 渲染区域尺寸
    const stageCanvas = stageCanvasRef.value!;
    const w = stageCanvas.clientWidth - padding.h;
    const h = stageCanvas.clientHeight - padding.v;

    // 先按宽度算高度
    targetW = w;
    targetH = w / ratio;

    // 如果高度超出，就用高度算宽度
    if(targetH > h){
        targetH = h;
        targetW = h * ratio;
    }

    canvasContainerRef.value!.style.width = `${targetW}px`;
    canvasContainerRef.value!.style.height = `${targetH}px`;

    playerSize.value.w = targetW;
    playerSize.value.h = targetH;

    if(mediaRef.value){
        (mediaRef.value as any).onParentResize();
    }
    
    // (mediaplayerRef.value as any).resize(targetW, targetH);
    // console.error('##########', mediaplayerRef as any, targetW, targetH)
    // console.error('!!!!!!!', targetW, targetH)
}

onMounted(() => {
    if (stageCanvasRef.value) {
        resizeObserver = new ResizeObserver(entries => {
            for (const entry of entries) {
                const { width, height } = entry.contentRect
                // containerSize.value = { width, height }
                console.error('📏 canvas 尺寸变了：', width, height)
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