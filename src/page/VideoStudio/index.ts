import { ref, Ref } from "vue";
import { AspecRatioItem } from '@src/components/VideoStudioToolbar';
import { PanelItem } from '@src/components/SlidingPanel';
import { 
    AudioPanel,
    ColorPanel,
    EffectPanel,
    FilterPanel
} from '@src/page/SliderBarRight/index';

import {
    SlidingPanel,
    VideoStudioToolbar,
    MediaPlayer,
    VideoPreview,
    TransformableLayer,
    TimeLine
} from '@src/components/index'

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
    // {
    //     id: 'layer-001',
    //     type: 'video',
    //     source: {
    //         id: 'video-001',
    //         type: 'video',
    //         uri: '/test2.mp4'
    //     },
    //     zIndex: 10,
    //     active: false
    // },
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
    // 视频编辑器初始化状态
    const init = ref(false);

    // 储存所有图层（视频、音频、图像、文本）的引用
    const layersRef = ref([]);

    const videoRef = ref([]);

    const stageCanvasRef = ref<HTMLElement | null>();
    const canvasContainerRef = ref<HTMLElement | null>();
    let resizeObserver: ResizeObserver | null = null;

    const mediaRef = ref(null);

    let targetW = 0, targetH = 0;

    let ratio = 16 / 9;

    // 右侧侧边栏菜单
    const rightSlidingItems: PanelItem[] = [
        { id: 0, label: '音频', icon: 'volume_up',      tooltip: 'test', component: AudioPanel },
        { id: 1, label: '滤镜', icon: 'filter_vintage', tooltip: 'test', component: FilterPanel },
        { id: 2, label: '效果', icon: 'contrast',       tooltip: 'test', component: EffectPanel },
        { id: 3, label: '颜色', icon: 'palette',        tooltip: 'test', component: ColorPanel } ,
    ];

    /**
     * 处理视频比例更新
     * @param item 
     */
    function handleRatioUpdate(item: AspecRatioItem){
        if(!item) return;

        const [w, h] = item.value?.split(':').map(Number)!;
        ratio = w / h;
        resizeCanvasContainer(ratio);
    }
    /**
     * 重置容器尺寸
     * @param ratio  目标比例, 比如 16/9
     */
    function resizeCanvasContainer(ratio: number){

        // padding
        const padding = { h: 100, v: 100 };
        // 渲染区域尺寸
        const stageCanvas = stageCanvasRef.value!;
        const w = stageCanvas.clientWidth - padding.h;
        const h = stageCanvas.clientHeight - padding.v;

        // 先按宽度算高度
        targetW = w;
        targetH = w / ratio;

        // 如果高度超出，就用高度算宽度
        if(targetH > h){
            targetH = h;
            targetW = h * ratio;
        }

        canvasContainerRef.value!.style.width = `${targetW}px`;
        canvasContainerRef.value!.style.height = `${targetH}px`;


        if(mediaRef.value){
            (mediaRef.value as any).onParentResize();
        }
        
        // (mediaplayerRef.value as any).resize(targetW, targetH);
        // console.error('##########', mediaplayerRef as any, targetW, targetH)
        // console.error('!!!!!!!', targetW, targetH)
    }

    function addResizeObserver(){
        if (stageCanvasRef.value) {
            resizeObserver = new ResizeObserver(entries => {
                for (const entry of entries) {
                    const { width, height } = entry.contentRect
                    // containerSize.value = { width, height }
                    console.error('📏 canvas 尺寸变了：', width, height)
                    resizeCanvasContainer(ratio)
                }
            })
    
            resizeObserver.observe(stageCanvasRef.value)
        }
    }

    function removeResizeObserver(){
        if (resizeObserver && canvasContainerRef.value) {
            resizeObserver.unobserve(canvasContainerRef.value)
            resizeObserver.disconnect()
        }
    }

    return {
        stageCanvasRef,
        canvasContainerRef,
        resizeObserver,
        init,
        videoRef,
        layersRef,
        layers,
        resizeCanvasContainer,
        handleRatioUpdate,
        rightSlidingItems,
        addResizeObserver,
        removeResizeObserver
    }
}

/**
 * 全局拖拽事件，用于高亮拖拽的元素
 * @returns 
 */
export function useDrag(){
    
    // 全局拖拽状态
    const globalDragging = ref(false);

    const draggingEnter = ref(false);

    function onDragStart(e: DragEvent){
        console.error('useGlobalDragState start')
        globalDragging.value = true;
        
    }

    function onDragEnd(e: DragEvent){
        console.error('useGlobalDragState end')
        globalDragging.value = false;
    }

    function addGlobalDragEvent(){
        window.addEventListener('dragstart', onDragStart);
        window.addEventListener('dragend', onDragEnd);
    }

    function removeGlobalDragEvent(){
        window.removeEventListener('dragstart', onDragStart);
        window.removeEventListener('dragend', onDragEnd);
    }

    function onDragEnter(e: DragEvent){
        draggingEnter.value = true;
    }

    function onDrop(e: DragEvent){
        draggingEnter.value = false;
        e.preventDefault();
        const data = e.dataTransfer?.getData('application/json');
        if(!data) return;

        try {
            const layer: ILayer = JSON.parse(data);
            console.log('接收到拖拽数据:', layer);
            // 添加到 layers 中
            layers.value.push(layer);
        } catch (e) {
            console.error('数据解析失败', e);
        }
    }

    function onDragLeave(e: DragEvent){
        draggingEnter.value = false;
    }
    
    return{
        globalDragging,
        draggingEnter,
        onDragEnter,
        onDrop,
        onDragLeave,
        addGlobalDragEvent,
        removeGlobalDragEvent
    }
}

export function useTimeline(){
    const playing = ref(false);     // 播放状态
    return{
        playing
    }
}

export {
    SlidingPanel,
    VideoStudioToolbar,
    MediaPlayer,
    VideoPreview,
    TransformableLayer,
    TimeLine
}