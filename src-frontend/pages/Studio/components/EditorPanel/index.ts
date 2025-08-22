import { AspecRatioItem } from '@frontend/components/VideoStudioToolbar';

import {
    TransformableLayer,
    TimeLine
} from '@frontend/components/index'

import { Asset } from "clip-engine";

import { VideoStudio } from '@frontend/store/videostudio';
import { uuidv4 } from 'zod';
import { 
    set_render_window_size,
    set_render_window_position
} from '@frontend/api/tauri-api';


export function useVideoStudio(){

    const videoRef = ref<HTMLCanvasElement>([]);

    const stageCanvasRef = ref<HTMLElement | null>();
    const canvasContainerRef = ref<HTMLElement | null>();
    let resizeObserver: ResizeObserver | null = null;

    const mediaRef = ref(null);

    let targetW = 0, targetH = 0;

    let ratio = 16 / 9;

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

        const rect = canvasContainerRef.value.getBoundingClientRect();
        const scale = window.devicePixelRatio;

        set_render_window_size(rect.width * scale, rect.height * scale);
        set_render_window_position(rect.x, rect.y * scale)
        
    }

    function addResizeObserver(){
        if (stageCanvasRef.value) {
            resizeObserver = new ResizeObserver(entries => {
                for (const entry of entries) {
                    const { width, height } = entry.contentRect;
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
        videoRef,
        resizeCanvasContainer,
        handleRatioUpdate,
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

    // 遮罩层
    const coverRef = ref<HTMLDivElement | null>(null);

    // 拖拽元素携带的数据
    const draggintData = ref<Asset>(null);

    // 拖拽元素在父容器中的尺寸
    let tw, th;

    // 拖拽元素在父容器中的位置
    let tx, ty;

    /**
     * 更新遮罩层样式
     */
    function updateCoverStyle(){
        if(!coverRef.value) return;

        if(!draggintData.value){
            console.error("没有拖拽元素数据");
            return;
        }

        // 获取父容器的宽高
        const parent = coverRef.value.parentElement as HTMLElement;
        
        // 保持拖拽元素的比例，在父容器中找到最大的尺寸
        const ratio = draggintData.value.width / draggintData.value.height;

        tw = parent.clientWidth;
        th = tw / ratio;

        if(th > parent.clientHeight1){
            th = parent.clientHeight;
            tw = th * ratio;
        }

        tx = (parent.clientWidth - tw) / 2;
        ty = (parent.clientHeight - th) / 2;

        coverRef.value.style.width = tw + "px";
        coverRef.value.style.height = th + "px";

        console.error('============ UpdateCoverStye', tx, ty)


    }

    function onDragStart(e: DragEvent){
        globalDragging.value = true;

        const data = e.dataTransfer?.getData('application/json');
        if(!data) return;

        const asset: Asset = JSON.parse(data);
        draggintData.value = asset;

        if(asset.width == 0 || asset.height == 0){
            console.error('拖拽元素未设置宽高属性');
        }
    }

    function onDragEnd(e: DragEvent){
        console.error('useGlobalDragState end')
        globalDragging.value = false;
        draggintData.value = null;
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
        
        updateCoverStyle();
    }

    function onDrop(e: DragEvent){
        draggingEnter.value = false;
        e.preventDefault();
        const data = e.dataTransfer?.getData('application/json');
        if(!data) return;

        // 添加轨道
        const asset: Asset = JSON.parse(data);
         
        // TODO: 添加到指定轨道
        VideoStudio.data.clipEngine.addVideoTrack(asset, { width: tw, height: th, x: tx, y: ty });
    }

    function onDragLeave(e: DragEvent){
        draggingEnter.value = false;
    }
    
    return{
        coverRef,
        globalDragging,
        draggingEnter,
        onDragEnter,
        onDrop,
        onDragLeave,
        addGlobalDragEvent,
        removeGlobalDragEvent,
        updateCoverStyle
    }
}

export function useTimeline(){
    const playing = ref(false);     // 播放状态
    return{
        playing
    }
}

export {
    TransformableLayer,
    TimeLine
}