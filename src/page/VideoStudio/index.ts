import { ref, Ref } from "vue";

interface IMediaSource {
    id: string;                                   // 唯一标识符
    type: "video" | "audio" | "image" | "text";   // 视频、音频、图片、文本
    uri: string;                                  // 本地路径,网络 URL，Blob URL
    text?: string;                                // 当 type == text 时该字段有效
}

interface ILayer{
    id: string;
    type: 'video' | 'audio' | 'image' | 'text';
    source: IMediaSource;
    zIndex: number;
    active: boolean;
}

const layers: Ref<ILayer[]> = ref([
    {
        id: 'layer-001',
        type: 'video',
        source: {
            id: 'video-001',
            type: 'video',
            uri: '/test2.mp4'
        },
        zIndex: 10,
        active: true
    },
    // {
    //     id: 'layer-002',
    //     type: 'video',
    //     source: {
    //         id: 'video-001',
    //         type: 'video',
    //         uri: '/test4.mp4'
    //     },
    //     zIndex: 2,
    //     active: false
    // },
    // {
    //     id: 'layer-003',
    //     type: 'image',
    //     source: {
    //         id: 'image-01',
    //         type: 'image',
    //         uri: '/tauri.svg'
    //     },
    //     zIndex: 11,
    //     active: false
    // },
    // {
    //     id: 'layer-004',
    //     type: 'text',
    //     source: {
    //         id: 'text-01',
    //         type: 'text',
    //         uri: '',
    //         text: 'HelloWorld'
    //     },
    //     zIndex: 12
    // }
]);

export function useVideoStudio(){
    return {
        layers
    }
}