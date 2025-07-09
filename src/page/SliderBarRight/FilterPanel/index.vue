<template>
    <div class="flex flex-col gap-2 p-2 overflow-y-scroll h-full">
        <div v-for="(filter, index) in filter_list" :key="index"
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
import { defineOptions, onMounted, ref } from 'vue';
import { AXIOS } from '@api';
import { Filter } from '@src-shared';
defineOptions({ name: 'FilterPanel' });

const filter_list = ref<Filter>([]);

onMounted(async() => {
    try{
        const res = await AXIOS.request({
            method: 'GET',
            url: '/api/filter/query',
            headers:{
                'Content-Type': 'multipart/form-data'
            }
        });
        filter_list.value = res.data;
        console.error('Filter List:', filter_list.value);
    }catch(e){

    }
});
</script>