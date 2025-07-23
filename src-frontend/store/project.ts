import { z } from 'zod';
import { ref } from 'vue';
import { ProjectSchema, TrackSchema, TrackType, ClipSchema } from 'clip-engine';
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
        addVideoTrack(){
            Project.methods.addTrack('video');
        },
        addAudioTrack(){
            Project.methods.addTrack('audio');
        },
        addTrack(type: TrackType){
            // TODO: 添加轨道的同时，默认添加一个音视频源
            const input: z.input<typeof TrackSchema> = {
                id: `${uuidv4()}`,
                name: `${type}-${Project.data.project?.tracks.length}`,
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

            console.error(TrackSchema.parse(input));
            Project.data.project?.tracks.push(input);
        },
        delTrack(name: string){

        }
    }
});

export const Project = ProjectRef.value;