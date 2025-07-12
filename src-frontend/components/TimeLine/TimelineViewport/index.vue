<template>
    <div>
        <VideoTrack v-for="(track, index) in videoTracks" :key="index" class="w-full h-[52px] bg-red-300 mb-[4px]"/>
    </div>
</template>

<script setup lang="ts">
import { ref } from "vue";
import VideoTrack from './VideoTrack/index.vue'

const videoTracks = ref([
  {
    id: 'track-1',
    clips: [
      { id: 'clip-1', startTime: 0, duration: 3, src: 'a.mp4', type: 'video' },
      { id: 'clip-2', startTime: 4, duration: 2, src: 'b.mp4', type: 'video' },
    ],
  },
  {
    id: 'track-2',
    clips: [
      { id: 'clip-3', startTime: 2, duration: 5, src: 'c.mp4', type: 'video' },
    ],
  },
])
</script>