<template>
    <div :class="['flex flex-row', placement=='left'? '' : 'flex-row-reverse']">
        <div class="flex flex-col space-y-2 items-center w-[72px]">
            <div v-for="(item, index) in items" :key="index" @click="handleClick(item)" :class="['relative group flex flex-col items-center justify-center text-[11px] text-gray-700 w-[58px] h-[58px] p-2  rounded-[10px] cursor-pointer transition-all duration-300 ease-in-out',
                    activedItem.id == item.id? 'bg-[#FAFAFF]' : 'hover:bg-[#F5F5FA]'
                ]">
                <!-- 高亮块容器 -->
                <div :class="['absolute top-0 h-full w-[3px] flex items-center justify-center', hlBlockStyle]">
                    <!-- 实际高亮块 -->
                    <div :class="['w-full bg-[#892fff] rounded-full',
                            activedItem.id == item.id? 'h-[16px] group-hover:h-[20px]' : 'group-hover:h-[3px] h-[0px]']">
                    </div>
                </div>
                <!-- 图标 -->
                <span class="material-symbols-outlined">{{ item.icon }}</span>
                <!-- 文本 -->
                <span class="text-[10px]">{{ item.label }}</span>
            </div>
        </div>
        <div 
            class="h-full bg-white rounded-[15px] shadow-md transition-all duration-300 overflow-hidden"
            :style="{ width: isExpanded ? '300px' : '0px' }"
        >
            <div class="w-[300px] shrink-0">
                <!-- drawer 标题 -->
                <div class="w-full h-[40px] font-bold text-[15px] pt-[20px] pl-[16px] pr-[16px] mb-[16px] flex flex-row justify-between items-center">
                    <span>{{  activedItem.label }}</span>
                    <span class="material-symbols-outlined cursor-pointer"
                        @click="isExpanded = false"    
                    >menu_open</span>
                </div>
                <!-- drawer 内容区域 -->
                <component class="w-full h-full" :is="activedItem.component" />
            </div>
        </div>
    </div>

</template>

<script setup lang="ts">
/**
 * TODO: 
 * 1、实现 drawer 滑入画出的效果
 */
// 数据示例
// const leftSlidingItems: PanelItem[] = [
//     { id: 0, label: '媒体',   icon: 'movie_info',             tooltip: 'test', component: 'MediaVault'},
//     { id: 1, label: '内容库', icon: 'books_movies_and_music', tooltip: 'test', component: 'TypeFoundry'},
//     { id: 2, label: '文字',   icon: 'text_fields',            tooltip: 'test', component: 'MediaVault'},
//     { id: 3, label: '模板',   icon: 'stacks',                 tooltip: 'test', component: 'MediaVault'},
// ]
import { defineOptions, defineProps, defineEmits, ref, withDefaults, computed, watch } from 'vue';
import { PanelItem } from './index';
import MediaVault from '@src/page/LeftSliderBar/MediaVault/index.vue'

defineOptions({ name: 'SlidingPanel' });
const props = withDefaults(defineProps<{
    placement?: 'left' | 'right',
    expanded?: boolean,
    items: PanelItem[]
}>(), {
    expanded: true,
    placement: 'right',
});
const emit = defineEmits<{
    (e: 'update:expanded', val: boolean): void
}>();

// 高亮块样式
const hlBlockStyle = computed(() => {
    return props.placement === 'left' ? 'right-full' : 'left-full';
});

const isExpanded = ref(props.expanded);

const activedItem = ref<PanelItem>(props.items[0]);

function handleClick(item: PanelItem){
    if(activedItem.value.id === item.id){
        isExpanded.value = !isExpanded.value
    }else{
        isExpanded.value = true;
    }
    activedItem.value = item;
}

watch(isExpanded, (val) => {
    emit('update:expanded', val);
})

</script>

<style>
</style>