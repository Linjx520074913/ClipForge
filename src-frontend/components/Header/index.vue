<template>
    <div class="flex flex-row justify-end items-center">
        <div class="w-[96px] h-[32px] font-bold text-[14px] flex flex-row justify-center items-center rounded-[4px] bg-gray-200 mr-[10px]">
            <span class="material-symbols-outlined text-[#8B4D00] mr-[3px]">diamond</span>
            <span>升级</span>
        </div>
        <div class="w-[96px] h-[32px] rounded-[4px] flex flex-row space-x-1 justify-center items-center mr-[50px] text-white bg-purple">
            <span>导出</span>
            <span class="material-symbols-outlined">download</span>
        </div>
        <!-- 头像 -->
        <span class="avatar w-[32px] h-[32px] rounded-full bg-blue-200 flex justify-center items-center font-bold">L</span>
    </div>
</template>

<script setup lang="ts">
import { defineOptions } from 'vue';

defineOptions({ name: 'Header' });

</script>