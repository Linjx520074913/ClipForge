<template>
    <div class="flex-row-end-center font-bold text-[14px]">
        <div 
            class="flex-row-center-center w-[96px] h-[32px] rounded-[4px] bg-gray-200 mr-[10px]"
        >
            <span class="material-symbols-outlined text-[#8B4D00] mr-[3px]">diamond</span>
            <span>升级</span>
        </div>
        <div class="flex-row-center-center w-[96px] h-[32px] px-[12px] py-[5px] rounded-[4px] space-x-1 text-[#BCBCCD]">
            <div class="w-full flex-row-center-center">
                导出
                <span class="material-symbols-outlined">download</span>
            </div>
        </div>
        <!-- 头像 -->
        <img 
            class="flex-row-center-center w-[32px] h-[32px] rounded-full bg-blue-200"
            :src="avatarUrl"
        />
    </div>
</template>

<script setup lang="ts">
defineOptions({ name: 'StudioTopbar' });

const avatarUrl = ref('/imgs/avata.png');

</script>