<template>
    <div class="flex-row-end-center font-bold">
        <div 
            class="flex-row-center-center w-[96px] h-[32px] text-[14px] rounded-[4px] bg-gray-200 mr-[10px]"
        >
            <span class="material-symbols-outlined text-[#8B4D00] mr-[3px]">diamond</span>
            <span>升级</span>
        </div>
        <div class="flex-row-center-center w-[96px] h-[32px] rounded-[4px] space-x-1 mr-[50px] text-white bg-purple">
            <span>导出</span>
            <span class="material-symbols-outlined">download</span>
        </div>
        <!-- 头像 -->
        <span class="flex-row-center-center w-[32px] h-[32px] rounded-full bg-blue-200">L</span>
    </div>
</template>

<script setup lang="ts">
defineOptions({ name: 'StudioTopbar' });

</script>