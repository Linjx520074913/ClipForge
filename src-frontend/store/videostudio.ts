import { computed, ref } from "vue";
import { ClipEngine, TimeDriver } from "clip-engine";
import { Project } from './project';

type onTimeTick = (timeMs: number) => void;

const VideoStudioRef = ref({
    data:{
        clipEngine: null as ClipEngine | null,
        curTimeMs: 0
    },
    methods:{
        async initialize(tick: onTimeTick){
            
            function initProject(){
                Project.methods.create('TestSample');
                Project.methods.addVideoTrack();
                Project.methods.addVideoTrack();
                Project.methods.addAudioTrack();
            }

            async function initClipEngine(tick: onTimeTick){
                VideoStudio.data.clipEngine = await ClipEngine.create();

                const data = VideoStudio.data;
                data.clipEngine.on('time:start', (timeMs: number) => {
                });
                data.clipEngine.on('time:pause', (timeMs: number) => {
                });
                data.clipEngine.on('time:stop', (timeMs: number) => {
                });
                data.clipEngine.on('time:tick', (timeMs: number) => {
                    data.curTimeMs = timeMs;
                    tick(timeMs);
                });
            }

            initProject();
            await initClipEngine(tick); 
        },
        start(){
            VideoStudio.data.clipEngine!.getTimeDriver().start();
        },
        pause(){
            VideoStudio.data.clipEngine!.getTimeDriver().pause();
        },
        stop(){
            VideoStudio.data.clipEngine!.getTimeDriver().stop();
        },
        async seek(timeMs: number){
            VideoStudio.data.clipEngine!.getTimeDriver().seek(timeMs);
        }
    }
});

const VideoStudio = VideoStudioRef.value;

export{
    VideoStudio
}