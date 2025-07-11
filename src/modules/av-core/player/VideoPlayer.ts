import { WebGPURenderer } from "../renderer/WebGPURenderer";
import { IPlayer, PlayerEvent } from "./IPlayer";
import { createPlayer } from "./PlayerFactory";
import { IFilter } from '@src-shared';

export class VideoPlayer{
    private player: IPlayer;

    private renderer: WebGPURenderer;

    constructor(canvas: HTMLCanvasElement, size: { w: number, h: number}){
        this.renderer = new WebGPURenderer(canvas, size);
        this.player = createPlayer('webav', this.renderer);
    }

    async load(src: string | ReadableStream<Uint8Array>): Promise<void>{
        await this.player.load(src);
    }

    setFilters(filters: IFilter[]){
        this.player.setFilters(filters);
    }

    play(): void{
        this.player.play();
    }

    pause(): void{
        this.player.pause();
    }

    stop(): void{
        this.player.stop();
    }

    seek(timeUs: number): void{
        this.player.seek(timeUs);
    }

    getCurrentTimeUs(): number{
        return this.player.getCurrentTimeUs();
    }

    getDurationUs(): number{
        return this.player.getDurationUs();
    }

    on(event: PlayerEvent, cb: () => void): void{
        this.player.on(event, cb);
    }

    destroy(): void{
        this.player.destroy();
    }

    setOuterSize(outerWidth: number, outerHeight: number): void{
        this.player.setOuterSize(outerWidth, outerHeight);
    }

    resize(width: number, height: number): void{
        this.player.resize(width, height);
    }
}