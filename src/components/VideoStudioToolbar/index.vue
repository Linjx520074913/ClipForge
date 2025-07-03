<template>
    <div class="flex flex-row cursor-pointer z-10">
        <!-- 下拉菜单 -->
        <div class="w-3/5 flex items-center justify-center relative hover:bg-[#F1F1F2] rounded-md" 
            ref="dropdownRef"
            @click="isActived = !isActived">
            <span class="material-symbols-outlined mr-1">crop_free</span>
            <span class="text-[14px] font-bold">大小</span>
            <span class="material-symbols-outlined">keyboard_arrow_down</span>
            <!-- 下拉菜单 -->
            <ul v-if="isActived" class="absolute left-0 top-full mt-1 w-[230px] bg-[#FAFAFF] shadow-md p-1">
                <li v-for="(item, index) in aspectRatios" :key="index" 
                    :class="['p-[1px] rounded-[5px]', item.divider ? 'pointer-events-none select-none' : 'hover:bg-[#EEEEF3] cursor-pointer']" 
                    @click="activeRatio(item)">
                    <div class="flex flex-row items-center mt-2 mb-2 ml-2" v-if="!item.divider">
                        <!-- 打钩 -->
                        <div class="mt-1 mr-1 w-[20px]">
                            <span class="material-symbols-outlined" v-if="activedRatio.value == item.value">done</span>
                        </div>
                        <!-- 图标 -->
                        <span class="material-symbols-outlined mr-1">{{ item.icon }}</span>
                        <!-- 描述文字 -->
                        <div class="flex flex-col">
                            <span class="text-[13px]">{{  item.label }}</span>
                            <span class="text-gray-500 text-[12px]">{{ item.desc }}</span>
                        </div>
                    </div>
                    <div v-if="item.divider" class="w-ful h-[1px] bg-gray-300 mt-3 mb-3"/>
                </li>
            </ul>
        </div>
        <div class="w-1/5 hover:bg-[#F1F1F2] rounded-md  grid place-items-center">
            <span class="material-symbols-outlined">search</span>
        </div>
        <div class="w-1/5 hover:bg-[#F1F1F2] rounded-md grid place-items-center">
            <span class="material-symbols-outlined">notifications</span>
        </div>
    </div>
</template>
  
<script setup lang="ts">
/**
 * TODO: 
 * 1、添加选项选中事件，供外部使用
 * 2、添加下拉菜单动画
 */
import { onMounted, onBeforeUnmount, defineOptions, defineEmits } from 'vue';
import { AspectRatioItem, useAspectRatio } from './index';

defineOptions({ name: 'VideoStudioToolbar' })
const emit = defineEmits<{
    (e: 'update:ratio', ratio: AspectRatioItem): void
}>();

const { isActived, activedRatio, aspectRatios, dropdownRef } = useAspectRatio();

function activeRatio(item: AspecRationItem){
    activedRatio.value = item;
    emit('update:ratio', item);
}
const handleClickOutside = (event: MouseEvent) => {
    const target = event.target as Node;
    if(dropdownRef.value && !dropdownRef.value.contains(target)){
        isActived.value = false;
    }
}

onMounted(() => {
    activeRatio(aspectRatios[0]);
    document.addEventListener('click', handleClickOutside);
})

onBeforeUnmount(() => {
    document.removeEventListener('click', handleClickOutside);
})
</script>
  