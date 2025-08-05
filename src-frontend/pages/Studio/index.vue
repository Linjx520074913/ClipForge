<template>
    <div class="flex flex-row">
        <!-- 主体区域：左右三栏布局 -->
        <AssetsPanel/>
        <div class="resize w-[7px] h-full"/>
        <EditorPanel class="flex-1"/>
        <div class="resize w-[7px] h-full"/>
        <InspectorPanel/>
    </div>
</template>

<script setup lang="ts">
import {
    AssetsPanel,
    EditorPanel,
    InspectorPanel
} from './components/index';

defineOptions({ name: 'Studio' });

</script>