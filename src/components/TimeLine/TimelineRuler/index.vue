<template>
    <div class="relative">
        <div class="w-full h-full overflow-x-scroll" ref="containerRef"
            @scroll="onScroll">
            <canvas ref="canvasRef" @mousemove="mouseMove" />
        </div>
        <div ref="indicatorRef" 
            class="absolute w-[4px] h-[200px] bg-black z-60 top-[0px]"
            :style="{ left: indicatorPosX + 'px', willChange: 'left' }"
            @mousedown="isActived = true"
            @mouseup="isActived = false">
            <div :class="['absolute w-[20px] h-[20px] bg-black transform -translate-x-1/2 transition-transform duration-300 ease-in-out', isActived?  'scale-110':'']"></div>
            <div v-if="isActived" class="absolute w-[60px] h-[20px] bg-red-200  left-[-30px] top-[-25px] rounded-md grid place-content-center transition-transform duration-300">
                {{ curTime }}
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
import { defineOptions, defineProps, onMounted, ref } from "vue";

defineOptions({ name: 'TimelineRuler' })

const props = defineProps<{
    /** 视频总时长 */
    duration: number 
}>();

const containerRef = ref < HTMLDivElement | null>();
const canvasRef    = ref<HTMLCanvasElement | null>();
const indicatorRef = ref<HTMLDivElement | null>();
let ctx: CanvasRenderingContext2D;

let indicatorPosX = ref(0);

let lengthFactor = 1.5;
let w = 0, h = 0;

// 大刻度间隔 30 s
const tickInterval = 30;

// 一秒有多少像素
let pxPerSec = 0;

const isActived = ref(false);

const curTime = ref('0')

function drawRuler() {
    const canvas = canvasRef.value;
    const container = containerRef.value?.getBoundingClientRect()!;
    if (!canvas) return;

    // 设备像素比
    const dpr = window.devicePixelRatio || 1;

    // 默认时间轴的长度为父容器的 lengthFactor 倍，这样可以看到全部时长，还有部分空余
    w = container.width * dpr * lengthFactor;
    h = container.height* dpr * lengthFactor;


    canvas.width = w;
    canvas.height = h;

    ctx = canvas.getContext('2d')!;
    // 保证清晰度缩放
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    if(!ctx) return;

    // 视频时间总刻度占父容器的 3/4, 算出有一秒有多少个像素
    pxPerSec = container.width * window.devicePixelRatio * 0.75 / props.duration;

    // 大刻度间隔像素值
    const tickPx = pxPerSec * tickInterval;
    // 总时长可以划分多少个大刻度
    const tickCnt = props.duration / tickInterval + 1;
    // 每个 10 秒绘制一个刻度
    for(let i = 0; i < tickCnt; i++){
        let x = i * tickPx;
        let y = h * 0.2;
        drawLine(ctx, 2, x, 0, x, y);
        drawText(ctx, (i * tickInterval).toString(), x, h * 0.4);
    } 
}

function mouseMove(e: any){
    if(!isActived.value) return;

    // 计算鼠标相对画布的位置
    const mouseX = e.clientX - canvasRef.value!.getBoundingClientRect().left - containerRef.value!.scrollLeft;

    // 计算出当前的秒数
    const realX = e.clientX - canvasRef.value!.getBoundingClientRect().left;

    const t = realX / pxPerSec;
    if(t <= props.duration){
        indicatorPosX.value = mouseX;
        curTime.value = t.toFixed(0);
    }
}

function drawText(ctx: CanvasRenderingContext2D, text: string, x: number, y: number, color: string = '#F00'){
    ctx.fillStyle = color;
    ctx.textAlign = 'center';
    ctx.font = '12px Arial';
    ctx.fillText(text, x, y);
}

function drawLine(ctx: CanvasRenderingContext2D, lineWidth: number, x1: number, y1: number, x2: number, y2: number, color: string = '#F00'){
    ctx.beginPath();
    ctx.strokeStyle = color;
    ctx.lineWidth = lineWidth;
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();
}

// 滚动的时候保持指示头不动
function onScroll(){
    indicatorPosX.value = Number(curTime.value) * pxPerSec - containerRef.value!.scrollLeft;
}


onMounted(() => {
    if(canvasRef.value){
        drawRuler();
    }
});
</script>