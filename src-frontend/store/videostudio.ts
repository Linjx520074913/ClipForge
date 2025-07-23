import { computed, ref } from "vue";
import { ClipEngine, TimeDriver } from "clip-engine";

type onTimeTick = (timeMs: number) => void;

const VideoStudioRef = ref({
    data:{
        clipEngine: null as ClipEngine | null,
        curTimeMs: 0
    },
    methods:{
        async initialize(tick: onTimeTick){
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
        },
        start(){
            console.error('======start');
            VideoStudio.data.clipEngine!.getTimeDriver().start();
        },
        pause(){
            console.error('======pause');
            VideoStudio.data.clipEngine!.getTimeDriver().pause();
        },
        stop(){
            console.error('======stop');
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