<template>
    <FoldableSlidebar :direction="'left'" :items="leftSlidingItems"/>
</template>

<script setup lang="ts">
defineOptions({ name: 'AssetsPanel' });

import AudioPanel from './AudioPanel/index.vue';
import MediaPanel from './MediaPanel/index.vue';
import FontPanel  from './FontPanel/index.vue';

import { FoldableSlidebar } from '@frontend/components';
const leftSlidingItems: PanelItem[] = [
    { id: 0, label: '您的媒体', icon: 'movie_info',             tooltip: 'test', component: MediaPanel },
    { id: 1, label: '内容库',   icon: 'books_movies_and_music', tooltip: 'test', component: AudioPanel },
    { id: 2, label: '文字',     icon: 'text_fields',            tooltip: 'test', component: FontPanel },
    { id: 3, label: '模板',     icon: 'stacks',                 tooltip: 'test', component: AudioPanel },
]
</script>