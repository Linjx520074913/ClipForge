<template>
    <div class="flex flex-row w-full">
        <div class="flex flex-col flex-1 overflow-hidden min-w-[300px] rounded-[15px] shadow-sm border bg-white">
            <!-- 主渲染区域 -->
            <div class="stage-canvas w-full h-3/4 relative border-b border-b-gray-300 grid place-items-center" 
                ref="stageCanvasRef">
                <!-- 顶部工具条 -->
                <VideoStudioToolbar 
                        class="absolute w-[158px] h-[34px] rounded-[5px] shadow-[0_3px_10px_rgba(0,0,0,0.12)] top-2 left-1/2 -translate-x-1/2"
                        @update:ratio="handleRatioUpdate"/>
                <!-- 预览区域,根据比例重置过大小 -->
                <div class="canvas-container bg-black relative overflow-hidden" ref="canvasContainerRef"
                    @dragover.prevent
                    @drop="onDrop"
                    @dragenter="onDragEnter"
                    @dragleave="onDragLeave">

                    <!-- 这一层 div 是用来显示拖拽的时候的效果的 -->
                    <div :class="['w-full h-full relative', globalDragging? 'pointer-events-none' : '']">
                        <!-- 生成 layer 层, layer 中包含 Video/Audio/Text/Image 等 -->
                        <TransformableLayer
                            v-for="(layer, index) in layers" :key="index" 
                            v-if="canvasContainerRef"
                            v-model:selected="layer.active"
                            :zIndex="layer.zIndex"
                            :to="'.stage-canvas'"
                            ref="layersRef"
                            :size="layer.size"
                            :pos="layer.pos"
                            >
                            <template #content>
                                <VideoPlayer 
                                    ref="videoRef" 
                                    v-if="layer.type == 'video'"
                                    :src="layer.source.uri" 
                                    @mousedown="layer.active = true"
                                    :size="layer.size"
                                    :engine="engine"/>
                                <img class="object-contain w-full h-full" 
                                    v-if="layer.type == 'image'" 
                                    :src="layer.source.uri" 
                                    @mousedown="layer.active = true"
                                    draggable="false"/>
                                <!-- TODO: 文字的拉伸有问题 -->
                                <div v-if="layer.type == 'text'" @mousedown="layer.active = true">{{ layer.source.text }} </div>
                            </template>
                        </TransformableLayer>

                        <!-- 拖拽进入的遮罩效果层 -->
                        <div
                            ref="coverRef" 
                            v-show="draggingEnter" 
                            class="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 bg-purple opacity-80 z-30 dash-border"/>
                    </div>
                    
                </div>
            </div>
            <!-- 时间轴 -->
            <TimeLine class="w-full" 
                v-model:playing="playing"
                :timeDriver="timeDriver"/>
        </div>
        <!-- 分割线 -->
        <div class="resize w-[7px] h-full"></div>
        <!-- 右侧面板 -->
        <SlidingPanel class="h-full" :items="rightSlidingItems" :expanded="true"
            @onChildEvent="handleRightSlidingPanelEvent"
            @onUpdateShader="onUpdateShader"/>
    </div>
    
</template>

<script setup lang="ts">

import { 
    useTimeline, 
    useVideoStudio,
    useDrag,
    SlidingPanel,
    VideoStudioToolbar,
    TransformableLayer,
    TimeLine,
    VideoPlayer,
    useClipEngine,
} from './index';

import { ClipEngine, VideoTrack, ILayer } from 'clip-engine';

import { ShaderDescription } from 'clip-engine';

import { defineOptions, ref, onMounted, onBeforeUnmount, watch, Ref } from 'vue';
defineOptions({ name: 'VideoStudio' });

let {
    init,
    layers,
    videoRef,
    layersRef,
    canvasContainerRef, 
    stageCanvasRef, 
    resizeObserver,
    resizeCanvasContainer,
    handleRatioUpdate,
    rightSlidingItems,
    addResizeObserver,
    removeResizeObserver
} = useVideoStudio();

const { playing } = useTimeline();


const { 
    coverRef,
    globalDragging,
    draggingEnter,
    onDragEnter,
    onDrop,
    onDragLeave, 
    addGlobalDragEvent, 
    removeGlobalDragEvent
} = useDrag();

let { engine, initClipEngine, timeDriver } = useClipEngine();


let preShaderDescs: ShaderDescription[] = [];

function diffFilters(prev: ShaderDescription[], next: ShaderDescription[]) {
    const added = next.filter(n => !prev.some(p => p.name === n.name));
    const removed = prev.filter(p => !next.some(n => n.name === p.name));
    return { added, removed };
}

function handleRightSlidingPanelEvent(desc: ShaderDescription[]){
    
    const { added, removed } = diffFilters(preShaderDescs, desc);

    if (added.length) {
        console.log('新增 filters:', added[0]);
        videoRef.value.forEach((instance: any, i: any) => {
            instance.addEffect(added[0]);
        });
    }
    if (removed.length) {
        console.log('移除 filters:', removed);
        videoRef.value.forEach((instance: any, i: any) => {
            instance.removeEffect(removed[0]);
        });
    }

    preShaderDescs = [...desc];
}

function onUpdateShader(s: ShaderDescription){
    console.error('########onupdateShader')
    videoRef.value.forEach((instance: any, i: any) => {
        instance.updateEffect(s);
    })
}


watch(() => playing, (val: Ref<boolean>) => {
    if(!init.value || engine.value == null) return;

    // 所有视频组件实例
    // videoRef.value.forEach((instance: any, i: any) => {
    //     console.error(instance, i);
    //     if(val.value){
    //         // 开始播放
    //         // TODO: 
    //         // 1、替换成 WebGPU 播放器
    //         // 2、使用滤镜效果
            
    //         instance.play();
    //     }else{
    //         // 暂停播放
    //         // instance.pause();
    //         instance.pause();
    //     }
    // })
    if(val.value){
        engine.value.getTimeDriver().start();
    }else{
        engine.value.getTimeDriver().pause();
    }
    
}, { deep: true, immediate: true });

onMounted(async () => { 
    init.value = true;
    await initClipEngine();
    addResizeObserver();
    addGlobalDragEvent();
});
onBeforeUnmount(() => {
    removeResizeObserver();
    removeGlobalDragEvent();
});
</script>

<style scoped>
.dash-border {
    border-width: 3px;
    border-style: solid;
    border-image-source: repeating-linear-gradient(135deg, #092FFF 0 6px, transparent 6px 12px);
    border-image-slice: 1;
}
</style>