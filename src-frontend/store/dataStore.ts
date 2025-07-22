import { computed, ref } from "vue";
import { ClipEngine, TimeDriver } from "clip-engine";

const VideoStudioRef = ref({
    data:{
        clipEngine: null as ClipEngine | null
    },
    methods:{
        async initialize(){
            VideoStudio.data.clipEngine = await ClipEngine.create();
        }
    }
});

const VideoStudio = VideoStudioRef.value;

export{
    VideoStudio
}