import { computed, ref } from "vue";
import { ClipEngine, TimeDriver } from "clip-engine";

type onFrameTick = (clipID: string, trackID: string, frame: VideoFrame) => void;

const VideoStudioRef = ref({
  data: {
    clipEngine: null as ClipEngine | null,
    curTimeMs: 0,
  },
  methods: {
    async initialize( 
        frameTick: onFrameTick
    ) {
        /**
         * 初始化资源
         */
        function initAssets() {
            const assetManager = VideoStudio.data.clipEngine?.getAssetManager();

            // TODO: 这里的 asset 信息需要先解析出来
            assetManager?.register({
                id: "asset-0",
                type: "video",
                label: "雪",
                url: "/sample_2.mp4",
                duration: 20000,
                width: 1920,
                height: 1080,
            });
            assetManager?.register({
                id: "asset-1",
                type: "video",
                label: "兔子",
                url: "/sample_1.mp4",
                duration: 5000,
                width: 1280,
                height: 720,
            });
            assetManager?.register({
                id: "asset-2",
                type: "video",
                label: "雪",
                url: "/sample_0.mp4",
                duration: 37000,
                width: 1920,
                height: 1080,
            });
        }

        function initProject() {
            VideoStudio.data.clipEngine?.createProject("Test-Sample");
        }

        async function initClipEngine() {
            VideoStudio.data.clipEngine = await ClipEngine.create();

            const data = VideoStudio.data;
            data.clipEngine.on("time:start", (timeMs: number) => {});
            data.clipEngine.on("time:pause", (timeMs: number) => {});
            data.clipEngine.on("time:stop", (timeMs: number) => {});
            data.clipEngine.on("time:tick", (timeMs: number) => {
                data.curTimeMs = timeMs;
            });
            data.clipEngine?.on(
                "frame:update",
                (clipID: string, trackID: string, frame: VideoFrame) => {
                    if(frameTick && frame){
                        frameTick(clipID, trackID, frame);
                        frame.close();
                    }
                }
            );
        }

        await initClipEngine();
        initAssets();
        initProject();
    },
    start() {
      VideoStudio.data.clipEngine!.getTimeDriver().start();
    },
    pause() {
      VideoStudio.data.clipEngine!.getTimeDriver().pause();
    },
    stop() {
      VideoStudio.data.clipEngine!.getTimeDriver().stop();
    },
    async seek(timeMs: number) {
      VideoStudio.data.clipEngine!.getTimeDriver().seek(timeMs);
    },
  },
});

const VideoStudio = VideoStudioRef.value;

export{
    VideoStudio
}