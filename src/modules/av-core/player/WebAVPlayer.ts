import type { IPlayer, PlayerEvent } from './IPlayer';
import type { IRenderer } from '../renderer/IRenderer';
import { MP4Clip } from '@webav/av-cliper';

export class WebAVPlayer implements IPlayer {
  private clip?: MP4Clip;
  private renderer?: IRenderer;
  private playing: boolean = false;
  private startTime = 0;
  private durationUs = 0;
  private currentTimeUs = 0;
  private playbackRate = 1.0;
  private rafId = 0;

  private eventListeners = new Map<PlayerEvent, (() => void)[]>();

  async initialize(renderer: IRenderer): Promise<void> {
    this.renderer = renderer;
  }

  private emit(event: PlayerEvent): void {
    const listeners = this.eventListeners.get(event);
    if (listeners) {
      for (const listener of listeners) {
        listener();
      }
    }
  }

  async load(source: string | ReadableStream<Uint8Array>): Promise<void> {
    // const resp = typeof source === 'string' ? await fetch(source) : { body: source };
    if (typeof source === "string") {
      const resp = await fetch(source);
      this.clip = new MP4Clip(resp.body!);
      const { duration, width, height } = await this.clip.ready;
      this.durationUs = duration;
      this.seek(0);
      this.emit("ready");
    }
  }

  play(): void {
    if (this.playing) return;

    this.playing = true;
    this.startTime =
      performance.now() - this.currentTimeUs / 1000 / this.playbackRate;
    this.emit("play");
    this.loop();
  }

    private loop = async (once: boolean = false): Promise<void> => {
        if ((!this.playing || !this.clip || !this.renderer) && !once) return;

        const elapsedMs = performance.now() - this.startTime;
        const timeUs = Math.round(elapsedMs * 1000 * this.playbackRate);

        this.currentTimeUs = timeUs;

        const { state, video } = await this.clip.tick(timeUs);
        
        if (state === 'done') {
            console.error('##################################', state)
            this.stop();
            this.emit("ended");
            return;
        }

        if (video && state === 'success') {
            this.emit('updateFrame');
            await this.renderer.render(video);
            video.close();
        }

        // 如果 once 为 true，则不再循环
        if (!once) {
            this.rafId = requestAnimationFrame(() => this.loop(false));
        }
    }

  pause(): void {
    this.playing = false;
    if(this.rafId){
        cancelAnimationFrame(this.rafId);
    }
    this.emit("pause");
  }

  stop(): void {
    this.loop(true);
    this.pause();
    this.emit("stop");
  }

  seek(timeUs: number): void {
    this.currentTimeUs = timeUs;
    // if (this.playing) {
      this.startTime = performance.now() - timeUs / 1000 / this.playbackRate;
      console.error("Seek during play - startTime updated:", this.startTime);
    // }
    this.loop(true);
    console.error("Current time set to:", this.currentTimeUs);
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

  setPlaybackRate(rate: number): void {
    this.playbackRate = rate;
    if (this.playing) {
      this.startTime =
        performance.now() - this.currentTimeUs / 1000 / this.playbackRate;
    }
  }

  on(event: PlayerEvent, callback: () => void): void {
    if (!this.eventListeners.has(event)) {
      this.eventListeners.set(event, []);
    }
    this.eventListeners.get(event)!.push(callback);
  }

  destroy(): void {
    this.pause();
    this.clip?.destroy();
    this.renderer?.destroy();
    this.eventListeners.clear();
  }
}
