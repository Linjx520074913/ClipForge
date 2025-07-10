<template>
    <div class="flex flex-col space-y-[12px]">
        <div class="flex flex-row space-x-[1px] ml-[16px] mr-[16px] cursor-pointer text-white">
            <!-- 导入按钮 -->
            <div
                class="w-5/6 h-[32px] flex-x-center bg-[#892FFF] hover:bg-[#6200D5] font-bold text-[15px] rounded-l-[5px]">
                导入媒体
            </div>
            <span
                class="flex-1 material-symbols-outlined flex-x-center bg-[#892FFF] hover:bg-[#6200D5] rounded-r-[5px]">arrow_drop_down</span>
        </div>
        <!-- 过滤排列 -->
        <div class="ml-[16px] mr-[16px] flex flex-row justify-between items-center">
            <input type="checkbox" class="w-[26px] h-[18px] cursor-pointer" v-model="selectAll" />
            <div class="cursor-pointer text-gray-600 flex-x-center">
                <span class="w-[26px] h-[30px] material-symbols-outlined hover:bg-[#ECECF2] rounded-[5px] flex flex-row justify-center items-center">filter_list</span>
                <span class="w-[26px] h-[30px] material-symbols-outlined hover:bg-[#ECECF2] rounded-[5px] flex flex-row justify-center items-center">swap_vert</span>
            </div>
        </div>
        <!-- 分割线 -->
        <div class="w-full h-[1px] bg-gray-200"/>
        <!-- 测试视频 -->
        <div v-for="(item, index) in videoLayers" :key="index" class="px-[12px]">
            <div class="flex flex-col hover:bg-gray-200 p-2 rounded-[10px]">
                <video
                    muted
                    :src="item.source.uri"
                    class="w-full h-[150px] rounded-[10px] mb-1 cursor-grab active:cursor-grabbing"
                    @mouseenter="($event.target as HTMLVideoElement).play()"
                    @mouseleave="handleMouseLeave"
                    draggable="true"
                    @dragstart="(e) => handleDragStart(e, item)"
                    />
                <p class="text-[13px] text-gray-500">{{ item.label }}</p>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, ref, Ref } from 'vue';
import { ILayer } from '@src-shared';
defineOptions({ name: 'MediaPanel' });

const selectAll = ref(false);


const videoLayers: Ref<ILayer[]> = ref([
    {
        id: 'layer-001',
        type: 'video',
        label: '测试视频1',
        source: {
            id: 'video-001',
            type: 'video',
            uri: '/test2.mp4'
        },
        zIndex: 10,
        active: false
    },
    {
        id: 'layer-002',
        type: 'video',
        label: '测试视频2',
        source: {
            id: 'video-001',
            type: 'video',
            uri: '/test4.mp4'
        },
        zIndex: 2,
        active: false
    }
]);

function handleDragStart(event: DragEvent, item: ILayer){
    // dataTransfer 不能传对象，要先序列化
    event.dataTransfer?.setData('application/json', JSON.stringify(item));
}

function handleMouseLeave(e: any){
    (e.target as HTMLVideoElement).pause();
    (e.target as HTMLVideoElement).currentTime = 0;
}

</script>

<style scoped lang="scss">
</style>