<template>
    <div 
        class="grid grid-cols-1 gap-1 p-2 overflow-y-scroll h-full"
        id="filter-panel"
    >
    {{ VideoStudio.data.clipEngine?.project.curTrackIndex }}
        <div
            v-for="(track, tidx) in VideoStudio?.data?.clipEngine?.project?.tracks"
            :key="tidx"
            :class="[VideoStudio.data.clipEngine?.project.curTrackIndex == tidx ? '' : 'hidden']"
        >
            <div 
                v-for="(clip, cidx) in track.clips"
                :key="cidx"
                class="flex flex-row overflow-x-scroll w-full space-x-2 border p-2 cursor-pointer"
            >
                <div
                    v-if="clip.isEditing"
                    v-for="(effect, eidx) in clip.effects" :key="eidx"
                    :class="['border w-[50px] flex-shrink-0 p-1 text-[12px] rounded-md', clip.curEffectIndex == eidx? 'bg-purple' : 'border-gray-200']"
                    @click.stop="clip.curEffectIndex = eidx"
                >
                    <img src="/imgs/avata.png" class="rounded-md mb-1"/>
                    {{ effect.name }}
                </div>
            </div>
            <!-- 参数面板 -->
            <div
                v-if="curEffect" 
                v-for="(param, pidx) in curEffect.params.entries" :key="pidx"
            >
                <label class="text-sm">{{ param.label }}</label>
                <input
                    v-if="param.type === 'f32'"
                    type="range"
                    :step="param.step"
                    :min="param.min"
                    :max="param.max"
                    v-model.number="param.value"
                    @click.stop
                    class="w-full"
                />
                <span class="text-xs text-gray-500">{{ param.value }}</span>
            </div>
        </div>
        <div 
            v-for="(filter, index) in filter_list" :key="index"
            :class="[
                'flex flex-col justify-center items-center w-full min-h-[150px] border cursor-pointer rounded-lg overflow-hidden px-2 py-1',
                filter.actived ? 'bg-purple' : 'hover:bg-[#E3E3E8]'
            ]"
            @click="activeFilter(index)">
            <img class="w-full flex-1">
            <p>{{ filter.name }}</p>
        </div>
        
    </div>
</template>

<script setup lang="ts">
/**
 * TODO:
 * 1、添加 API，获取滤镜列表
 * 2、滤镜列表可以多选，选中返回列表
 */
import { AXIOS } from '@frontend/api';
import { VideoStudio } from '@frontend/store/videostudio';
import { ShaderSpecSchema } from 'clip-engine';
import { z } from 'zod';
import { computed } from 'vue';
import { v4 as uuidv4 } from 'uuid';
defineOptions({ name: 'FilterPanel' });
const emit = defineEmits(['onChildEvent', 'onUpdateShader']);

/**
 * 滤镜列表
 */
const filter_list = ref<typeof ShaderSpecSchema>([]);

/**
 * 激活/禁用滤镜 
 * @param index 滤镜索引
 */
function activeFilter(index: number){
    if(!filter_list.value) return;

    const project = VideoStudio.data.clipEngine?.project!;
    const curTrack = project.tracks[project.curTrackIndex];
    if(!curTrack) return undefined;

    const curClip = curTrack.clips[curTrack.curClipIndex];
    if(!curClip) return undefined;

    const spec: z.input<typeof ShaderSpecSchema> = { ...filter_list.value[index] };
    spec.id = uuidv4();
    curClip.effects.push(spec);

    VideoStudio.data.clipEngine?.render();
}

const curEffect = computed(() => {
    const project = VideoStudio.data.clipEngine?.project!;
    const curTrack = project.tracks[project.curTrackIndex];
    if(!curTrack) return undefined;

    const curClip = curTrack.clips[curTrack.curClipIndex];
    if(!curClip) return undefined;
    
    return curClip.effects[curClip.curEffectIndex];
});

onMounted(async() => {
    try{
        // 获取服务器端滤镜列表
        const res = await AXIOS.request({
            method: 'GET',
            url: '/api/filter/query',
            headers:{
                'Content-Type': 'multipart/form-data'
            }
        });
        filter_list.value = res.data;
        console.error('Filter List:', filter_list.value);
    }catch(e){
        console.error(e)
    }
});
</script>