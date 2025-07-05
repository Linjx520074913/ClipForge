import { Ref } from 'vue';

// 锚点样式
interface AnchorStyle {
    left:   string;
    top:    string;
    width:  string;
    height: string;
    zIndex: number;
}

/**
 * 更新锚点和控制框
 * root 锚点所在的元素容器
 * to
 * @returns 
 */
function updateAnchorStyle(root: Ref<HTMLElement | null>, props: any, style: Ref<AnchorStyle>) {
    if (!root.value) return;

    // 获取 rootRef 的位置
    const rect = root.value.getBoundingClientRect();
    
    // 获取 Teleport 目标容器的位置
    const teleportTarget = document.querySelector(props.to) as HTMLElement;
    const teleportRect = teleportTarget?.getBoundingClientRect() || { left: 0, top: 0 };
    
    // 更新控制框的样式
    style.value = {
        left: `${rect.left - teleportRect.left}px`,
        top: `${rect.top - teleportRect.top}px`,
        width: `${rect.width}px`,
        height: `${rect.height}px`,
        zIndex: props.zIndex
    };
}

export { updateAnchorStyle }

// 变换（缩放、移动）起点
let transformStart = { left: 0, top: 0, x: 0, y: 0, w: 0, h: 0 };

export function useResize(
    rootRef: Ref<HTMLElement | null>
){

    // 左上、右上、左下、右下四个锚点
    const cornerAnchors = ['tl', 'tr', 'bl', 'br'];

    // 锚点样式
    const anchorCls: Record<string, string> = {
        tl: 'top-0 left-0 -translate-x-1/2 -translate-y-1/2 cursor-nwse-resize',
        tr: 'top-0 right-0 translate-x-1/2 -translate-y-1/2 cursor-nesw-resize',
        bl: 'bottom-0 left-0 -translate-x-1/2 translate-y-1/2 cursor-nesw-resize',
        br: 'bottom-0 right-0 translate-x-1/2 translate-y-1/2 cursor-nwse-resize',
    };

    // 当前锚点
    let curAnchor: typeof cornerAnchors[number] | null = null;

    // 比例
    let ratio = 1;

    /**
     * 拖动锚点开始缩放
     * @param e 
     */
    function startResize(anchor: typeof cornerAnchors[number], e: MouseEvent){
        const root = rootRef.value;
        if(!root) return;

        curAnchor = anchor;

        transformStart = {
            left: root.offsetLeft,
            top:  root.offsetTop,
            w:    root.offsetWidth,
            h:    root.offsetHeight,
            x:    e.clientX,
            y:    e.clientY
        };

        ratio = transformStart.w / transformStart.h;

        window.addEventListener('mousemove', onResize);
        window.addEventListener('mouseup',   stopResize);
    }

    /**
     * 缩放
     * @param e 
     */
    function onResize(e: MouseEvent){
        const root = rootRef.value;
        if(!curAnchor || !root) return;
        

        const dx = e.clientX - transformStart.x;
        const dy = dx / ratio;

        let newW = transformStart.w;
        let newH = transformStart.h;

        switch(curAnchor){
            case 'tl':
                // 拖动左上角，改变 left，top
                newW = newW - dx;
                root.style.left = transformStart.left + dx + 'px';
                root.style.top = transformStart.top + dy + 'px';
                break;
            case 'tr':
                // 拖动右上角, left 不变，改变 top
                newW = newW + dx;
                root.style.top = transformStart.top - dy + 'px';
                break;
            case 'bl':
                // 拖动左下角，top 不变，改变 left
                newW = newW - dx;
                root.style.left = transformStart.left + dx + 'px';
                break;
            case 'br':
                // 拖动右下角，left 和 top 都不变
                newW = newW + dx;
                break;
            default:
                break;
        }
        newH = newW / ratio;

        root.style.width = `${newW}px`
        root.style.height = `${newH}px`;
    }

    /**
     * 停止缩放
     * @param e 
     */
    function stopResize(e: MouseEvent){
        window.removeEventListener('mousemove', onResize);
        window.removeEventListener('mouseup', stopResize);
        curAnchor = null
    }

    return{
        cornerAnchors,
        anchorCls,
        startResize
    }
}

export function useMove(
    props: any,
    emit: any,
    rootRef: Ref<HTMLElement | null>,
    anchorStyle: Ref<AnchorStyle>
){
    /**
     * 开始移动
     * @param e 
     * @returns 
     */
    function startMove(e: MouseEvent){
        const root = rootRef.value;
        if(!root) return;

        emit('update:selected', true);
        transformStart = {
            left: root.offsetLeft,
            top: root.offsetTop,
            x: e.clientX,
            y: e.clientY,
            w: root.offsetWidth,
            h: root.offsetHeight,
        };
        window.addEventListener('mousemove', onMove);
        window.addEventListener('mouseup', stopMove);
    }

    /**
     * 移动中
     * @param e 
     * @returns 
     */
    function onMove(e: MouseEvent){
        if(!rootRef.value) return;

        const dx = e.clientX - transformStart.x;
        const dy = e.clientY - transformStart.y;

        rootRef.value.style.left = `${transformStart.left + dx}px`;
        rootRef.value.style.top = `${transformStart.top + dy}px`;

        updateAnchorStyle(rootRef, props, anchorStyle);
    }

    /**
     * 停止移动
     */
    function stopMove(){
        window.removeEventListener('mousemove', onMove);
        window.removeEventListener('mouseup', stopMove);
    }

    return{
        startMove
    }
}