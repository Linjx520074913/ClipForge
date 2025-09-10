<template>
    <div 
        class="flex flex-col h-screen select-none box-border relative"
    >
        <AppHeader class="w-full h-[56px]" style="padding: 8px 12px 8px 16px"/>
        <Studio class="w-full flex-1 h-0 mb-[5px]"/>
    </div>
</template>

<script setup lang="ts">
/**
 * TODO
 * 1、所有交互添加提示，比如 按钮
 */
import { AppHeader } from '@frontend/components';
import { TestClipEngine } from '@frontend/test';
import { Studio } from '@frontend/pages';
</script>

<style>
.flex-x-center{
    @apply flex flex-row justify-center items-center;
}
.flex-col-center-center {
    @apply flex flex-col justify-center items-center;
}
.flex-row-center-center{
    @apply flex flex-row justify-center items-center;
}
.flex-row-end-center{
    @apply flex flex-row justify-end items-center;
}
.flex-row-between-center{
    @apply flex flex-row justify-between items-center;
}
</style>