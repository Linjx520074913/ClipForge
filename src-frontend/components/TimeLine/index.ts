import { ref } from "vue";

export function useTimeline(
    duration: number
) {

    const pxPerSec = 30; 

    const majorTick    = 5000;   // 毫秒，表示每隔 1 秒画一根主刻度
    const subDivisions = 4;      // 主刻度下的等分数

    const rulerRef = ref<HTMLCanvasElement | null>(null);

    function formatTime(ms: number){
        const t = ms / 1000;
        const min = Math.floor(t / 60);
        const sec = Math.floor(t % 60);

        const mm = String(min).padStart(1, '0');
        const ss = String(sec).padStart(2, '0');
        return `${mm}:${ss}`;
    }

    function drawRuler(){
        const ruler = rulerRef.value;
        if(!ruler) return;

        const dpr = window.devicePixelRatio || 1;
        ruler.width  = ruler.clientWidth * dpr;
        ruler.height = ruler.clientHeight* dpr;

        const ctx = ruler.getContext("2d");
        if(!ctx) return;
        ctx.scale(dpr, dpr);
        
        // TODO：选中轨道的时候，时间尺子才设置改颜色
        // ctx.clearRect(0, 0, ruler.width, ruler.height);
        // ctx.fillStyle = "#F5ECFF";
        // ctx.fillRect(0, 0, ruler.width, ruler.height);

        // 主刻度数量
        const majorTickCount = Math.ceil(duration / majorTick) + 1;

        for(let i = 0; i < majorTickCount; i++){
            // 主刻度，显示时间
            // 设置字体样式
            ctx.font = "10px sans-serif";         // 字号 + 字体
            ctx.fillStyle = "#000";               // 文字颜色
            ctx.textBaseline = "top";             // 对齐方式（top, middle, bottom, alphabetic 等）

            const textX = i * majorTick / 1000 * pxPerSec;
            // 绘制文字
            ctx.fillText(formatTime(i * majorTick), textX, ruler.clientHeight / 3);   // 参数：(内容, x, y)
            
            // 次刻度，显示灰色小圆点
            for (let t = 1; t < subDivisions; t++) {
                // 计算当前时间对应的像素位置
                const x = (i * majorTick + t * majorTick / subDivisions) / 1000 * pxPerSec ;
                ctx.fillStyle = "#DFDFE8";
                const radius = 2;
                ctx.beginPath();
                ctx.arc(x, ruler.clientHeight / 2, radius, 0, Math.PI * 2);
                ctx.fill();
            }
        }

    }

    return {
        pxPerSec,
        rulerRef,
        drawRuler
    }
}