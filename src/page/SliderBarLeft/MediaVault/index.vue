<template>
    <div class="flex flex-col space-y-[12px]">
        <div class="flex flex-row space-x-[1px] ml-[16px] mr-[16px] cursor-pointer text-white">
            <!-- 导入按钮 -->
            <div
                class="w-5/6 h-[32px] flex-x-center bg-[#892FFF] hover:bg-[#6200D5] font-bold text-[15px] rounded-l-[5px]">
                导入媒体
            </div>
            <span
                class="flex-1 material-symbols-outlined flex-x-center bg-[#892FFF] hover:bg-[#6200D5] rounded-r-[5px]">arrow_drop_down</span>
        </div>
        <!-- 过滤排列 -->
        <div class="ml-[16px] mr-[16px] flex flex-row justify-between items-center">
            <input type="checkbox" class="w-[26px] h-[18px] cursor-pointer" v-model="selectAll" />
            <div class="cursor-pointer text-gray-600 flex-x-center">
                <span class="w-[26px] h-[30px] material-symbols-outlined hover:bg-[#ECECF2] rounded-[5px] flex flex-row justify-center items-center">filter_list</span>
                <span class="w-[26px] h-[30px] material-symbols-outlined hover:bg-[#ECECF2] rounded-[5px] flex flex-row justify-center items-center">swap_vert</span>
            </div>
        </div>
        <!-- 分割线 -->
        <div class="w-full h-[1px] bg-gray-200"/>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, ref } from 'vue';
defineOptions({ name: 'MediaVault' });

const selectAll = ref(false);
</script>

<style scoped lang="scss">
</style>