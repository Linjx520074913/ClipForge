<template>
    <div class="flex flex-col gap-2 p-2 overflow-y-scroll h-full">
        <div v-for="(filter, index) in filters" :key="index"
            class="flex flex-col w-[150px] h-[150px] border">
            <img class="w-full flex-1">
            <p>{{ filter.label }}</p>
        </div>
    </div>
</template>

<script setup lang="ts">
/**
 * TODO:
 * 1、添加 API，获取滤镜列表
 * 2、滤镜列表可以多选，选中返回列表
 */
import { defineOptions } from 'vue';
import { useFilters } from './index';
defineOptions({ name: 'FilterPanel' });

const { filters } = useFilters();
</script>