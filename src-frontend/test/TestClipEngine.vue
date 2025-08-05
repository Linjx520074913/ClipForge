<template>
    <div class="p-4 select-none">
        <p>Test ClipEngine</p>
        <div class="flex flex-row space-x-4 mt-8">
            <div>
                <p>InputTexture</p>
                <img
                    ref="imageRef" 
                    class="border border-red-500 w-[400px] h-[400px] object-contain"
                    src="/imgs/test-lion.jpg"/>
            </div>
            <div class="w-[200px]">
                滤镜列表
                <ul>
                    <li v-for="(s, index) in shaders" :key="index" class="w-full mb-1 cursor-pointer">
                        <div @click.stop.prevent="activeShader(s)" 
                            :class="['font-bold flex justify-center items-center', s.actived? 'bg-purple' : 'bg-gray-300']">
                            {{ s.name }}
                        </div>
                        <div v-if="s.actived" 
                            v-for="(param, key) in s.params.entries" :key="key" :class="['flex flex-col p-1', s.actived? 'border border-gray-400' : '']">
                            <label class="text-sm">{{ param.label }}</label>
                            <input
                                v-if="param.type === 'f32'"
                                type="range"
                                :step="param.step"
                                :min="param.min"
                                :max="param.max"
                                v-model.number="param.value"
                                @input="onParamsChange(s)"
                                class="w-full"
                            />
                            <span class="text-xs text-gray-500">{{ param.value }}</span>
                        </div>
                    </li>
                </ul>
            </div>
            <div>
                <p>OutputTexture</p>
                <canvas
                    ref="canvasRef"
                    class="border border-red-500 w-[400px] h-[400px]"/>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue';

import {
    useTestClipEngine
}from './index';

const { 
    imageRef,
    canvasRef,
    init,
    activeShader,
    render,
    shaders,
    onParamsChange
} = useTestClipEngine();

onMounted(async () => {
    await init();
    render();
});

onUnmounted(() => {
    
});
</script>