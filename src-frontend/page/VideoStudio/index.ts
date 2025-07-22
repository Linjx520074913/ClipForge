import { ref, computed, watch } from "vue";
import { AspecRatioItem } from '@frontend/components/VideoStudioToolbar';
import { PanelItem } from '@frontend/components/SlidingPanel';
import { 
    AudioPanel,
    ColorPanel,
    EffectPanel,
    FilterPanel
} from '@frontend/page/SliderBarRight/index';

import {
    SlidingPanel,
    VideoStudioToolbar,
    TransformableLayer,
    TimeLine,
    VideoPlayer
} from '@frontend/components/index'

import { ClipEngine, ILayer } from "clip-engine";
import { TimeDriver } from "src-clip-engine/Engine/Time/TimeDriver";

const layers: Ref<ILayer[]> = ref([]);

export function useClipEngine(){
    
    const engine = ref<ClipEngine | null>(null);
    const timeDriver = ref<TimeDriver | null>(null);
    let videoRef = ref([]);

    const curTime = ref(0);

    async function initClipEngine(vf: ref[]){
        videoRef = vf;
        engine.value = await ClipEngine.create();
        timeDriver.value = engine.value.getTimeDriver();
        timeDriver.value.on('start', (time?: number) => {
            console.error('########### timeDriver start', time);
            curTime.value = time;
        });
        timeDriver.value.on('pause', (time?: number) => {
            console.error('########### timeDriver pause', time);
            curTime.value = time;
        });
        timeDriver.value.on('stop', (time?: number) => {
            console.error('########### timeDriver stop', time);
            curTime.value = time;
        });
        timeDriver.value.on('ended', (time?: number) => {
            console.error('########### timeDriver ended', time);
            curTime.value = time;
        });
        timeDriver.value.on('tick', async (timeMs: number) => {
            curTime.value = timeMs;
            console.error('================tick');
            videoRef.value.forEach(async (video: any, i: any) => {
                video.seek(timeMs * 1000);
            });
        });
    }

    function seek(timeMs: number){
        timeDriver.value?.seek(timeMs);
    }

    return{
        engine,
        curTime,
        seek,
        timeDriver,
        initClipEngine
    }
}

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
        { id: 0, label: '滤镜', icon: 'filter_vintage', tooltip: 'test', component: FilterPanel },
        { id: 1, label: '音频', icon: 'volume_up',      tooltip: 'test', component: AudioPanel },
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
        
    }

    function addResizeObserver(){
        if (stageCanvasRef.value) {
            resizeObserver = new ResizeObserver(entries => {
                for (const entry of entries) {
                    const { width, height } = entry.contentRect
                    // containerSize.value = { width, height }
                    // console.error('📏 canvas 尺寸变了：', width, height)
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

    // 遮罩层
    const coverRef = ref<HTMLDivElement | null>(null);

    // 拖拽元素携带的数据
    const draggintData = ref<ILayer>(null);

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
        const ratio = draggintData.value.source.width / draggintData.value.source.height;

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

        const layer: ILayer = JSON.parse(data);
        draggintData.value = layer;

        if(layer.source.width == 0 || layer.source.height == 0){
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

        try {
            const layer: ILayer = JSON.parse(data);
            layer.size.w = tw;
            layer.size.h = th;
            layer.pos.x  = tx;
            layer.pos.y  = ty;
            // 添加到 layers 中
            layers.value.push(layer);
            console.error('===============', layer)
        } catch (e) {
            console.error('数据解析失败', e);
        }
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
    SlidingPanel,
    VideoStudioToolbar,
    TransformableLayer,
    TimeLinemableLayer,
    TimeLine,
    VideoPlayer
}