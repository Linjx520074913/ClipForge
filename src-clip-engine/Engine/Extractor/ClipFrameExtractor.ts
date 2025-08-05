import { ClipWorkerRequest, ClipWorkerResponse } from ".";
import { FrameCache } from "./FrameCache";

export class ClipFrameExtractor {
    private worker: Worker;
    private init = false;

    private cache = new FrameCache(600);

    private frameDuration = 33;
    private windowSize = 5000;

    private pendingRequests = new Map<string, (frame: VideoFrame | null) => void>();

    private lastPreloadTime: number | null = null;

    constructor() {
        this.worker = new Worker(new URL('./ClipWorker.ts', import.meta.url), { type: 'module' });

        this.worker.addEventListener('message', (e: MessageEvent) => {
            const { type, frame, time, requestId } = e.data as ClipWorkerResponse;

            switch (type) {
                case 'init-done':
                    this.init = true;
                    console.error('------------------------------ init-done')
                    this.resolveRequest(requestId);
                    break;

                case 'frame':
                    
                    if (time !== undefined && frame) {
                        if (!this.cache.has(time)) {
                            this.cache.add(time, frame);
                            console.error('00000000000000000 time', time, frame)
                        } else {
                            frame.close();
                        }
                    }
                    this.resolveRequest(requestId, frame ? frame : null);
                    break;

                case 'dispose-done':
                    this.resolveRequest(requestId);
                    break;

                case 'error':
                    // 你可以这里抛错或者 reject promise
                    this.resolveRequest(requestId, null);
                    break;
            }
        });
    }

    private genRequestId(): string {
        return Math.random().toString(36).slice(2);
    }

    private resolveRequest(id?: string, frame?: VideoFrame | null) {
        if (id && this.pendingRequests.has(id)) {
            this.pendingRequests.get(id)!(frame ?? null);
            this.pendingRequests.delete(id);
        }
    }

    async initialize(url: string): Promise<void> {
        if (this.init) return;
        const id = this.genRequestId();
        return new Promise((resolve) => {
            this.pendingRequests.set(id, () => resolve());
            this.worker.postMessage({ type: 'init', url, requestId: id } as ClipWorkerRequest);
        });
    }

    async getFrame(time: number): Promise<VideoFrame | null> {
        if (!this.init) throw new Error('Extractor not init');

        // 尝试从缓存取
        const cached = this.cache.get(time);
        if (cached) return cached;

        // 新请求，先请求关键帧解码
        const id = this.genRequestId();
        const framePromise = new Promise<VideoFrame | null>((resolve) => {
            this.pendingRequests.set(id, resolve);
        });
        this.worker.postMessage({ type: 'get-frame', time, requestId: id } as ClipWorkerRequest);

        // 滑动窗口预解码控制，避免频繁重复调用
        console.error('HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH', this.lastPreloadTime === null || Math.abs(time - this.lastPreloadTime) > this.windowSize / 2);
        if (this.lastPreloadTime === null || Math.abs(time - this.lastPreloadTime) > this.windowSize / 2) {
            this.lastPreloadTime = time;
            setTimeout(() => {
                this.worker.postMessage({ type: 'preload-window', time } as ClipWorkerRequest);
            }, 100);
        }

        return framePromise;
    }

    async dispose(): Promise<void> {
        if (!this.init) return;
        const id = this.genRequestId();
        return new Promise((resolve) => {
            this.pendingRequests.set(id, () => resolve());
            this.worker.postMessage({ type: 'dispose', requestId: id } as ClipWorkerRequest);
        });
    }
}
