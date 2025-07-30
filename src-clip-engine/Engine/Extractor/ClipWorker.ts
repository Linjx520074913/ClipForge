import type { ClipWorkerRequest, ClipWorkerResponse } from './index';
import { MP4Clip } from "@webav/av-cliper";

let clip: MP4Clip | null = null;
let queue: { t: number, requestId: number }[] = [];
let isDecoding = false;

const WINDOW_SIZE = 5000;  // 预解码5秒
const FRAME_DURATION = 33;

let currentRequestTime: number | null = null;

async function initClip(url: string) {
    const resp = await fetch(url);
    clip = new MP4Clip(resp.body!);
    await clip.ready;
}

async function decodeFrame(t: number): Promise<VideoFrame | null> {
    if (!clip) return null;
    const { state, video } = await clip.tick(t * 1000);
    return (state === 'success' && video) ? video : null;
}

function cleanupQueue() {
    if (currentRequestTime === null) return;
    queue = queue.filter(item => item.t >= currentRequestTime!);
}

async function processQueue() {
    if (isDecoding || queue.length === 0) return;
    isDecoding = true;

    const { t, requestId } = queue.shift()!;
    const frame = await decodeFrame(t);

    const msg: ClipWorkerResponse = {
        type: 'frame',
        frame: frame ?? null,
        time: t,
        requestId
    };

    if (frame) {
        console.error('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%', t, requestId)
        self.postMessage(msg, [frame]);
    } else {
        self.postMessage(msg);
    }

    isDecoding = false;
    cleanupQueue();
    processQueue(); // 继续处理后续任务
}

function enqueue(t: number, requestId: number) {
    if (!queue.some(item => item.t === t && item.requestId === requestId)) {
        queue.push({ t, requestId });
        queue.sort((a, b) => a.t - b.t);
    }
}

self.onmessage = async (e: MessageEvent<ClipWorkerRequest>) => {
    const msg = e.data;

    switch (msg.type) {
        case 'init':
            await initClip(msg.url!);
            self.postMessage({ type: 'init-done', requestId: msg.requestId });
            break;

        case 'get-frame':
            currentRequestTime = msg.time!;
            queue = []; // 跳转清空旧队列
            enqueue(msg.time!, msg.requestId);
            processQueue();
            break;

        case 'preload-window':
            console.error('^^^^^^^^^^^^^', currentRequestTime, msg.time)
            currentRequestTime = msg.time!;
            
            for (let t = Math.max(currentRequestTime, msg.time!); t <= msg.time! + WINDOW_SIZE; t += FRAME_DURATION) {
                console.error('^^^^^^^^^^^^^', t)
                enqueue(t, msg.requestId);
            }
            processQueue();
            break;

        case 'dispose':
            clip?.destroy();
            clip = null;
            queue = [];
            currentRequestTime = null;
            self.postMessage({ type: 'dispose-done', requestId: msg.requestId });
            break;
    }
};
