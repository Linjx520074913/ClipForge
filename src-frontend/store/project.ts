import { z } from 'zod';
import { ref } from 'vue';
import { ProjectSchema, TrackSchema, TrackType, ClipSchema, Asset } from 'clip-engine';
import { v4 as uuidv4 } from 'uuid';

const ProjectRef = ref({
    data:{
        project: null as null | ReturnType<typeof ProjectSchema.parse>
    },
    methods:{
        create(name: string){
            const input: z.input<typeof ProjectSchema> = {
                id: uuidv4(),
                name: name,
                setting: {},
                tracks: [],
                createdAt: new Date().toISOString(),
                updatedAt: new Date().toISOString(),
            };
            Project.data.project = ProjectSchema.parse(input);
        },
        addVideoTrack(asset: Asset){
            Project.methods.addTrack('video', asset);
        },
        addAudioTrack(asset: Asset){
            Project.methods.addTrack('audio', asset);
        },
        addTrack(type: TrackType, asset: Asset){
            // TODO: 添加轨道的同时，默认添加一个音视频源
            const trackId = `${type}-${Project.data.project?.tracks.length}`;
            const trackInput: z.input<typeof TrackSchema> = {
                id: trackId,
                name: trackId,
                type: type,
                order: Project.data.project?.tracks.length,
                clips: [],  // 空剪辑列表
                isLocked: false,
                isVisible: true,
                isMuted: false,
                volume: 1,
                opacity: 1,
                blendMode: 'normal',
                effects: [] // 空特效数组
            };
            
            // 添加 clip
            const clipId = `${trackId}-clip-0`;
            const clipInput: z.input<typeof ClipSchema> = {
                id: clipId,
                name: clipId,
                type: type,
                assedId: asset.id, // TODO: 替换为真实资源 ID
                trackId: trackId,

                startTime: 0,
                duration: asset.duration,

                trim: {
                    startTime: 0,
                    endTime: 10000,
                    offset: 0
                },
                speed: {
                    value: 1.0
                },
                transformation: {
                    position: {
                        x: 0,
                        y: 0,
                        origin: 'top-left'
                    },
                    scale: {
                        x: 1,                          // 水平缩放（1.0 = 100%）
                        y: 1,                          // 垂直缩放（1.0 = 100%）
                        uniform: true                    // 是否保持宽高比
                    },
                    rotation: 0,                       // 旋转角度（度）
                    opacity:  0,                       // 透明度（0.0 = 0%， 1.0 = 100%）
                    crop: z.object({
                        top:    0,
                        bottom: 0,
                        left:   0,
                        right:  0
                    })
                }

            };
            trackInput.clips.push(clipInput);
            Project.data.project?.tracks.push(trackInput);
        },
        delTrack(name: string){

        }
    }
});

export const Project = ProjectRef.value;