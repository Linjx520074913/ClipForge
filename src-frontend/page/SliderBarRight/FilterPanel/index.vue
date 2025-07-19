<template>
    <div class="grid grid-cols-1 gap-1 p-2 overflow-y-scroll h-full">
        <div v-for="(filter, index) in filter_list" :key="index"
            :class="['flex flex-col justify-center items-center w-full h-[150px] border cursor-pointer rounded-lg overflow-hidden px-2 py-1',
                filter.actived ? 'bg-purple' : 'hover:bg-[#E3E3E8]'
            ]"
            @click="activeFilter(index)">
            <img class="w-full flex-1">
            <p>{{ filter.name }}</p>
            <div v-if="filter.actived" 
                v-for="(param, key) in filter.params.entries" :key="key" 
                :class="['flex flex-col p-1 w-full', filter.actived? 'border border-gray-400' : '']">
                <label class="text-sm">{{ param.label }}</label>
                <input
                    v-if="param.type === 'f32'"
                    type="range"
                    :step="param.step"
                    :min="param.min"
                    :max="param.max"
                    v-model.number="param.value"
                    @input.stop.prevent="onParamsChange(filter)"
                    @click.stop
                    class="w-full"
                />
                <span class="text-xs text-gray-500">{{ param.value }}</span>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
/**
 * TODO:
 * 1、添加 API，获取滤镜列表
 * 2、滤镜列表可以多选，选中返回列表
 */
import { defineOptions, defineEmits, onMounted, ref } from 'vue';
import { AXIOS } from '@frontend/api';
import { ShaderDescription } from 'clip-engine';
defineOptions({ name: 'FilterPanel' });
const emit = defineEmits(['onChildEvent', 'onUpdateShader']);

/**
 * 滤镜列表
 */
const filter_list = ref<ShaderDescription>([]);

const activeFilters: Filter[] = [];

/**
 * 激活/禁用滤镜 
 * @param index 滤镜索引
 */
function activeFilter(index: number){
    if(!filter_list.value) return;

    filter_list.value[index].actived = !filter_list.value[index].actived;
    
    // 如果是激活状态，则添加到激活列表
    if(filter_list.value[index].actived){
        activeFilters.push(filter_list.value[index]);
    }else{
        // 如果是禁用状态，则从激活列表中移除
        const idx = activeFilters.findIndex(f => f.id === filter_list.value[index].id);
        if(idx !== -1){
            activeFilters.splice(idx, 1);
        }
    }

    emit('onChildEvent', activeFilters);
}

function onParamsChange(s: ShaderDescription){
    console.error('@@@@@@@@', s)
    emit('onUpdateShader', s);
}

onMounted(async() => {
    try{
        // 获取服务器端滤镜列表
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
        console.error(e)
    }
});
</script>