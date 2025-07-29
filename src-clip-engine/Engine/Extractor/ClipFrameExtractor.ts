import { ClipWorkerRequest, ClipWorkerResponse } from ".";
import { FrameCache } from "./FrameCache";

export class ClipFrameExtractor {
    private worker: Worker;
    private init: boolean = false;

    private cache: FrameCache = new FrameCache(600);

    private frameDuration = 33;
    private preloadCount = 30;

    private pendingRequests = new Map<string, (frame: VideoFrame | null) => void>();

    constructor() {
        this.worker = new Worker(new URL('./ClipWorker.ts', import.meta.url), { type: 'module'});
    
        this.worker.addEventListener('message', (e: MessageEvent) => {
            const { type, frame, time, requestId } = e.data as ClipWorkerResponse;

            switch(type){
                case 'init-done':
                    this.init = true;
                    if(requestId && this.pendingRequests.has(requestId)){
                        this.pendingRequests.get(requestId)!();
                        this.pendingRequests.delete(requestId);
                    }
                    break;
                case 'frame':
                    {
                        if(time != undefined && frame) {
                            if(!this.cache.has(time)) {
                                this.cache.add(time, frame);
                            } else {
                                frame.close();
                            }
                        }

                        if(requestId && this.pendingRequests.has(requestId)) {
                            this.pendingRequests.get(requestId)!(frame ? frame.clone() : null);
                            this.pendingRequests.delete(requestId);
                        }
                    }
                    break;
                case 'dispose-done':
                    break;
                case 'error':
                    break;
                default:
                    break;
            }
            
        });
    }

    private genRequestId(): string {
        return Math.random().toString(36).slice(2);
    }

    async initialize(url: string): Promise<void> {
        if(this.init) return;

        return new Promise((resolve, reject) => {
            const requestId = this.genRequestId();
            this.pendingRequests.set(requestId, resolve);
            this.worker.postMessage({ type: 'init', url, requestId } as ClipWorkerRequest);
        });
    }

    async getFrame(time: number): Promise<VideoFrame | null> {
        if(!this.init) throw new Error('Extractor not init');
    
        const cached = this.cache.get(time);
        if(cached) {
            return cached.clone();
        }

        return new Promise((resolve, reject) => { 
            const requestId = this.genRequestId();
            this.pendingRequests.set(requestId, resolve);
            this.worker.postMessage({ type: 'get-frame', time, requestId } as ClipWorkerRequest);
        });
    }

    /**
     * 预解码未来帧
     * @param time 
     */
    preload(time: number) {
        return new Promise(() => {
            for (let i = 1; i <= this.preloadCount; i++) {
                const t = time + i * this.frameDuration;
                if (!this.cache.has(t)) {
                    this.worker.postMessage({ type: 'get-frame', time: t } as ClipWorkerRequest);
                }
            }
        });
    }

    async dispose(): Promise<void> {
        if(!this.init) return;

        return new Promise((resolve, reject) => {
            const requestId = this.genRequestId();
            this.pendingRequests.set(requestId, resolve);
            this.worker.postMessage({ type: 'dispose', requestId } as ClipWorkerRequest);
        });
    }
}