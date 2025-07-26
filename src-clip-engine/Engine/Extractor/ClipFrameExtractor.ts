import { ClipWorkerResponse } from ".";

export class ClipFrameExtractor {
    private worker: Worker;
    private init: boolean = false;

    constructor() {
        this.worker = new Worker(new URL('./ClipWorker.ts', import.meta.url), { type: 'module'});
    }

    async initialize(url: string): Promise<void> {
        if(this.init){
            
        }

        return new Promise((resolve, reject) => { 
            const initHandler = (e: MessageEvent) => {
                this.worker.removeEventListener('message', initHandler);

                const { type } = e.data as ClipWorkerResponse;
                if(type === 'init-done'){
                    this.init = true;
                    resolve();
                }else if(type === 'error') {
                    reject(e.data.msg);
                }
            };

            this.worker.addEventListener('message', initHandler);
            this.worker.postMessage({ type: 'init', url: url });
        });
    }

    async getFrame(time: number): Promise<VideoFrame | null> {
        if(!this.init) throw new Error('Extractor not init');
    
        return new Promise((resolve, reject) => {
            const frameHandler = (e: MessageEvent) => {
                const { type, frame } = e.data as ClipWorkerResponse;
                if(type === 'frame') {
                    this.worker.removeEventListener('message', frameHandler);
                    resolve(frame);
                }
            };
            
            this.worker.addEventListener('message', frameHandler);
            this.worker.postMessage({ type: 'get-frame', time: time });
        });
    }

    async dispose(): Promise<void> {
        if(!this.init) return;

        return new Promise((resolve) => {
            const disposeHandler = (e: MessageEvent) => {
                this.worker.removeEventListener('message', disposeHandler);
                this.init = false;
                resolve();
            };
            this.worker.addEventListener('message', disposeHandler);
            this.worker.postMessage({ type: 'dispose' });
        })
    }
}