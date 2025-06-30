<template>
    <div class="flex flex-col bg-[#f0f0f5]">
        <!-- 头部 -->
        <Header class="w-full h-[56px] py-[8px] pl-[16px] pr-[12px]" />
        <!-- 主内容区 -->
        <div class="flex flex-row flex-1 mb-[10px]">
            <!-- 左侧面板 -->
            <SlidingPanel class="h-full" :placement="'left'" :items="leftSlidingItems"/>
            <!-- 分割线 -->
            <div class="resize w-[7px] h-full"></div>
            <!-- 视频编辑区 -->
            <VideoStudio class="flex-1 h-full" />
        </div>
    </div>
</template>

<script setup lang="ts">
/**
 * TODO:
 * 1、分割线可以拖动改变左右区域尺寸
 */
import { defineOptions } from 'vue';
import { Header } from '@src/components/index';
import VideoStudio from '../VideoStudio/index.vue';

import {
    SlidingPanel,
    VideoStudioToolbar
} from '@src/components/index';
import { AspecRatioItem } from '@src/components/VideoStudioToolbar';
import { PanelItem } from '@src/components/SlidingPanel';
import {
    AssetLibrary,
    MediaVault,
    TemplateLibrary,
    TypeFoundry
} from '@src/page/SliderBarLeft/index';

defineOptions({ name: 'HomePage' });

const leftSlidingItems: PanelItem[] = [
    { id: 0, label: '您的媒体', icon: 'movie_info',             tooltip: 'test', component: MediaVault },
    { id: 1, label: '内容库',   icon: 'books_movies_and_music', tooltip: 'test', component: AssetLibrary },
    { id: 2, label: '文字',     icon: 'text_fields',            tooltip: 'test', component: TypeFoundry },
    { id: 3, label: '模板',     icon: 'stacks',                 tooltip: 'test', component: TemplateLibrary },
]

</script>

<style scoped>

</style>