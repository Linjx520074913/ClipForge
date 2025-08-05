<template>
    <div>
        <FoldableSlidebar 
            :direction="'right'" 
            :items="options"
            v-if="VideoStudio.data.clipEngine?.project?.tracks?.length"
        />
    </div>
</template>

<script setup lang="ts">
import { FoldableSlidebar } from '@frontend/components/index';
import AudioPanel  from './AudioPanel/index.vue';
import ColorPanel  from './ColorPanel/index.vue';
import EffectPanel from './EffectPanel/index.vue';
import FilterPanel from './FilterPanel/index.vue';

import { VideoStudio } from '@frontend/store/videostudio';

defineOptions({ name: 'InspectorPanel' });

// 右侧侧边栏菜单
const options: PanelItem[] = [
    { id: 0, label: '滤镜', icon: 'filter_vintage', tooltip: 'test', component: FilterPanel },
    { id: 1, label: '音频', icon: 'volume_up',      tooltip: 'test', component: AudioPanel },
    { id: 2, label: '效果', icon: 'contrast',       tooltip: 'test', component: EffectPanel },
    { id: 3, label: '颜色', icon: 'palette',        tooltip: 'test', component: ColorPanel } ,
];
</script>