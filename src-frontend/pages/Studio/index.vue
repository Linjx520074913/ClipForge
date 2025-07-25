<template>
    <div class="flex flex-row h-full">
        <!-- 主体区域：左右三栏布局 -->
        <AssetsPanel/>
        <div class="resize w-[7px] h-full"/>
        <EditorPanel class="flex-1"/>
        <div class="resize w-[7px] h-full"/>
        <InspectorPanel/>
    </div>
</template>

<script setup lang="ts">
import { VideoStudio } from '@frontend/store/videostudio';
import {
    AssetsPanel,
    EditorPanel,
    InspectorPanel
} from './components/index';

defineOptions({ name: 'Studio' });

onMounted(async () => {
    function onTick(timeMs: number) {
        
    }
    await VideoStudio.methods.initialize(onTick);
});

</script>