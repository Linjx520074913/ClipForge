export class TrackDecoder {
    private worker: Worker;
    private trackID: string;

    constructor(trackID: string, url){    
        this.worker = new Worker(new URL('./worker.ts', import.meta.url), { type: 'module' });
        this.worker.onmessage = this.onWorkerMessage.bind(this);
    }

    private onWorkerMessage(event: MessageEvent) {

    }

    async getFrame(timeMs: number): Promise {
        // 解码
        this.worker.postMessage({});
    }
}