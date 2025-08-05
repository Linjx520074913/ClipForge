export class FrameCache {

    private cache: Map<number, VideoFrame> = new Map();
    private maxSize: number;

    constructor(maxSize: number = 60) {
        this.maxSize = maxSize;
    }

    add(time: number, frame: VideoFrame): void {
        if(this.cache.size >= this.maxSize) {
            const oldestKey = this.cache.keys().next().value;
            this.cache.get(oldestKey)?.close();
            this.cache.delete(oldestKey);
        }
        this.cache.set(time, frame);
        
    }

    has(time: number, tolerance: number = 60): boolean {
        for(const t of this.cache.keys()) {
            if(Math.abs(t - time) <= tolerance) return true;
        }
        return false;
    }

    get(time: number, tolerance: number = 60): VideoFrame | undefined {
        for(const [t, frame] of this.cache.entries()) {
            if(Math.abs(t - time) <= tolerance) {
                return frame.clone();
            }
        }
        return null;
    }

    clear(): void {
        for(const frame of this.cache.values()) {
            frame.close();
        }
        this.cache.clear();
    }
}