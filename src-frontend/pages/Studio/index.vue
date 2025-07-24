<template>
    <div class="flex flex-row px-[10px]">
        <!-- 主体区域：左右三栏布局 -->
        <AssetsPanel class="w-[322px] bg-[#FAFAFF]"/>
        <div class="resize w-[7px] h-full"/>
        <EditorPanel class="flex-1 bg-white"/>
        <div class="resize w-[7px] h-full"/>
        <InspectorPanel class="w-[322px] bg-[#FAFAFF]"/>
    </div>
</template>

<script setup lang="ts">
defineOptions({ name: 'Studio' });

import {
    AssetsPanel,
    EditorPanel,
    InspectorPanel
} from './components/index';

</script>