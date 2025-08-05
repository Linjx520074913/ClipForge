import { MP4Clip } from '@webav/av-cliper';

export type PlayerEvent =
    | 'ready'
    | 'play'
    | 'pause'
    | 'stop'
    | 'seek'
    | 'ended'
    | 'error'
    | 'updateFrame';

export interface IPlayer {
    load(source: string | ReadableStream<Uint8Array>): Promise<void>;
    play(): void;
    pause(): void;
    stop(): void;
    seek(timeUs: number): void;
    getCurrentTimeUs(): number;
    getDurationUs(): number;
    setPlaybackRate(rate: number): void;
    isPlaying(): boolean;
    destroy(): void;
    on(event: PlayerEvent, callback: (...args: any[]) => void): void;
}

export class MP4Player implements IPlayer {
    private clip?: MP4Clip;
    private playing = false;
    private startTime = 0;
    private durationUs = 0;
    private currentTimeUs = 0;
    private playbackRate = 1.0;
    private rafId = 0;

    private eventListeners = new Map<PlayerEvent, ((...args: any[]) => void)[]>();

    on(event: PlayerEvent, callback: (...args: any[]) => void): void {
        if (!this.eventListeners.has(event)) {
            this.eventListeners.set(event, []);
        }
        this.eventListeners.get(event)!.push(callback);
    }

    private emit(event: PlayerEvent, ...args: any[]): void {
        const listeners = this.eventListeners.get(event);
        if (listeners) {
            for (const listener of listeners) {
                listener(...args);
            }
        }
    }

    async load(source: string | ReadableStream<Uint8Array>): Promise<void> {
        if (typeof source === "string") {
            const resp = await fetch(source);
            this.clip = new MP4Clip(resp.body!);
            const { duration, width, height } = await this.clip.ready;
            this.durationUs = duration;
            this.seek(0);
            this.emit("ready", { width, height });
        } else {
            // 暂不支持 ReadableStream 处理
            console.warn("load() 暂未实现 ReadableStream 加载");
        }
    }

    play(): void {
        if (this.playing || !this.clip) return;
        this.playing = true;
        this.startTime = performance.now() - this.currentTimeUs / 1000 / this.playbackRate;
        this.emit("play");
        this.loop();
    }

    pause(): void {
        this.playing = false;
        if (this.rafId) {
            cancelAnimationFrame(this.rafId);
            this.rafId = 0;
        }
        this.emit("pause");
    }

    stop(): void {
        this.pause();
        this.currentTimeUs = 0;
        this.emit("stop");
    }

    seek(timeUs: number): void {
        this.currentTimeUs = timeUs;
        this.startTime = performance.now() - timeUs / 1000 / this.playbackRate;
        this.loop(true); // 播一次
        this.emit("seek");
    }

    setPlaybackRate(rate: number): void {
        this.playbackRate = rate;
        if (this.playing) {
            this.startTime = performance.now() - this.currentTimeUs / 1000 / this.playbackRate;
        }
    }

    isPlaying(): boolean {
        return this.playing;
    }

    getCurrentTimeUs(): number {
        return this.currentTimeUs;
    }

    getDurationUs(): number {
        return this.durationUs;
    }

    destroy(): void {
        this.pause();
        this.clip?.destroy();
        this.eventListeners.clear();
    }

    private loop = async (once = false): Promise<void> => {
        if ((!this.playing && !once) || !this.clip) return;

        const elapsedMs = performance.now() - this.startTime;
        const timeUs = Math.round(elapsedMs * 1000 * this.playbackRate);
        this.currentTimeUs = timeUs;

        try {
            const { state, video } = await this.clip.tick(timeUs);

            if (state === 'done') {
                this.stop();
                this.emit("ended");
                return;
            }

            if (state === 'success' && video) {
                this.emit("updateFrame", video);
                video.close();
            }
        } catch (err) {
            this.emit("error", err);
            this.stop();
        }

        if (!once) {
            this.rafId = requestAnimationFrame(() => this.loop());
        }
    }
}
