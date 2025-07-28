<template>
    <div class="flex flex-row w-full h-full">
        <div class="flex flex-col flex-1 min-w-[300px] h-full rounded-[15px] shadow-sm border bg-white">
            <!-- 主渲染区域 -->
            <div 
                class="stage-canvas w-full h-3/4 relative border-b border-b-gray-300 grid place-items-center" 
                ref="stageCanvasRef"
            >
                <!-- 顶部工具条 -->
                <EditorToolbar 
                    class="absolute w-[158px] h-[34px] rounded-[5px] shadow-[0_3px_10px_rgba(0,0,0,0.12)] top-2 left-1/2 -translate-x-1/2"
                    @update:ratio="handleRatioUpdate"
                />
                <!-- 预览区域,根据比例重置过大小 -->
                <div
                    class="canvas-container bg-black relative overflow-hidden"
                    ref="canvasContainerRef"
                    @dragover.prevent
                    @drop="onDrop"
                    @dragenter="onDragEnter"
                    @dragleave="onDragLeave"
                >

                    <!-- 这一层 div 是用来显示拖拽的时候的效果的 -->
                    <div :class="['w-full h-full relative', globalDragging? 'pointer-events-none' : '']">
                        <!-- 每个 Clip 都对应一个 TransformableLayer -->
                        <div
                            v-for="(track, tidx) in VideoStudio.data.clipEngine?.project?.tracks" :key="tidx"
                        >
                            <!-- TODO resize 的时候要同步修改 size 和 pos -->
                            <TransformableLayer
                                v-for="(clip, cidx) in track.clips" :key="cidx"
                                v-if="canvasContainerRef"
                                v-model:selected="clip.isEditing"
                                :zIndex=track.order
                                :to="'.stage-canvas'"
                                :size="clip.transformation.size"
                                :pos="clip.transformation.position"
                                :class="[clip.isVisible? '' : 'hidden']"
                                :trackId="track.id"
                                :clipId="clip.id"
                                @onStatusChange="onStatusChange"
                            >
                                <template #content>
                                    <canvas 
                                        ref="videoRef" :id="`${track.id}:${clip.id}`"
                                        :style="{width: `${clip.transformation.size.w}px`, height: `${clip.transformation.size.h}px`}"
                                    />
                                </template>

                            </TransformableLayer>
                        </div>

                        <!-- 拖拽进入的遮罩效果层 -->
                        <div
                            ref="coverRef" 
                            v-show="draggingEnter" 
                            class="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 bg-purple opacity-80 z-30 dash-border"
                        />
                    </div>
                    
                </div>
            </div>
            <!-- 时间轴 -->
            <TimeLine
                class="w-full h-1/4" 
                v-model:playing="playing"
                :curTime="VideoStudio.data.curTimeMs"
                @onSeek="VideoStudio.methods.seek"
            />
        </div>
    </div>
</template>

<script setup lang="ts">
import { VideoStudio } from '@frontend/store/videostudio';
import { ShaderDescription } from 'clip-engine';
import { 
    useTimeline, 
    useVideoStudio,
    useDrag,
    TransformableLayer,
    TimeLine
} from './index';

import EditorToolbar from './EditorToolbar/index.vue';

defineOptions({ name: 'EditorPanel' });

let {
    videoRef,
    canvasContainerRef, 
    stageCanvasRef, 
    resizeObserver,
    resizeCanvasContainer,
    handleRatioUpdate,
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

function onStatusChange(data: {trackId: string, clipId: string, value: boolean}){
    console.error('@@@@@@ onStatusChange @@@@@@', data)
    // 获取/丢失焦点
    const project = VideoStudio.data.clipEngine?.project;
    if(!project) return;

    project.curTrackIndex = -1;

    project.tracks.forEach(
        (track, tidx) => {
            if(track.id == data.trackId)    {
                track.isEditing = data.value;
            }
            if(track.isEditing){
                project.curTrackIndex = tidx;
            }
            track.clips.forEach(
                (clip, cidx) => {
                    if(clip.id == data.clipId) {
                        clip.isEditing = data.value;
                    }
                    
                    if(clip.isEditing){
                        track.curClipIndex = cidx;
                    }
                }
            );
        }
    );
}

/**
 * 控制 TimeDriver 的播放状态
 * @param playing 
 */
watch(() => playing, (val: Ref<boolean>) => {

    if(val.value){
        VideoStudio.methods.start();
    }else{
        VideoStudio.methods.pause();
    }
    
}, { deep: true });

watch(
    () => [...videoRef.value],
    (newCanvases, oldCanvases = []) => {
        // 1. 生成ID映射（假设每个canvas有唯一id）
        const newIds = new Set(newCanvases.map(c => c.id));
        const oldIds = new Set(oldCanvases.map(c => c.id));

        // 2. 找出新增元素（在新不在旧）
        const added = newCanvases.filter(c => !oldIds.has(c.id));
        
        // 3. 找出删除元素（在旧不在新）
        const removed = oldCanvases.filter(c => !newIds.has(c.id));
       
        if(added.length){
            // 添加 clip 轨
            added.forEach((instance: any, i: any) => {
                const id = instance.id.split(':');
                const trackId = id[0];
                const clipId = id[1];
                VideoStudio.data.clipEngine?.bindClipCanvasToTrack(trackId, clipId, instance);
            });
        }

        if(removed.length){
            // 删除 clip 轨
        }
    },
    { deep: true, flush: 'post' }
);

onMounted(async () => {
    function onFrameTick(clipID: string, trackID: string, frame: VideoFrame) {
        videoRef.value.forEach((instance: any, i: any) => {
            instance.updateFrame(frame);
        })
    }
    await VideoStudio.methods.initialize(onFrameTick);
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