import { ref, computed } from 'vue';

export function useMouse(props: any, emit: any){
    const actived = ref(false);
    const rootRef = ref();

    // 一秒有多少像素
    let pxPerSec = 30;
    // 当前时间对应的位置
    let pos = computed(() => {
        return `${props.timeMs / 1000 * pxPerSec}px`;
    });

    function onMouseDown(e: MouseEvent){
        actived.value = true;
        window.addEventListener('mousemove', onMouseMove);
        window.addEventListener('mouseup',   onMouseUp);
        document.body.style.cursor = 'grabbing';
        emit('start-seek');
    }

    function onMouseMove(e: MouseEvent){
        if(!actived.value) return;

        // 获取播放头的父节点，计算在父节点中的偏移
        const parent = rootRef.value.parentElement;
        if(!parent) return;

        const offsetX = e.clientX - parent.getBoundingClientRect().left - parent.scrollLeft;
        
        // 计算当前秒数
        let time = (offsetX / pxPerSec) * 1000;
        time = time > 0 ? time : 0;
        emit('update:time-ms', time);
        emit('on-seek', time);
    }

    function onMouseUp(e: MouseEvent){
        actived.value = false;
        document.body.style.cursor = '';
        window.removeEventListener('mousemove', onMouseMove);
        window.removeEventListener('mouseup', onMouseUp);
        emit('end-seek');
    }
    return{
        actived,
        rootRef,
        onMouseDown,
        pos
    }
}