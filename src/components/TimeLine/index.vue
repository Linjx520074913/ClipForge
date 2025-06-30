<template>
    <div class="flex flex-col w-full h-[410px] border-red-400 border">
        <TimelineControls class="w-full h-[41px] bg-blue-200"/>
        <TimelineRuler 
            :duration="10"
            class="w-full h-[80px] bg-green-200"
        />
        <TimelineViewport class="w-full flex-1 bg-blue-300"/>
    </div>
</template>

<script setup lang="ts">
import TimelineControls from './TimelineControls/index.vue'
import TimelineRuler    from './TimelineRuler/index.vue'
import TimelineViewport from './TimelineViewport/index.vue'
</script>   