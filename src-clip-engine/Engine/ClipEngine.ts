import { TrackRenderer } from './TrackRenderer';
import { GPUContext } from './GPUContext';
import { BaseTrack } from './Track/BaseTrack';
import { TimeDriver } from './Time/TimeDriver';
import { EngineEvent, EventBus } from './EventBus';
import { AssetManager } from './AssetManager';
import { getAsset } from 'node:sea';
class TrackGraph{

}

export class ClipEngine{

    private ctx: GPUContext;
    // private trackGraph: TrackGraph;
    private timeDriver: TimeDriver;

    private assetManager: AssetManager;

    private eventBus: EventBus<EngineEvent>;
    

    private tracks: BaseTrack[] = [];

    private constructor(ctx: GPUContext){
        this.ctx = ctx;
        this.timeDriver = new TimeDriver(30000);
        this.assetManager = new AssetManager();
        this.eventBus = new EventBus<EngineEvent>();

        this.timeDriver.on('start', (time) => this.eventBus.emit('time:start', time));
        this.timeDriver.on('pause', (time) => this.eventBus.emit('time:pause', time));
        this.timeDriver.on('stop',  (time) => this.eventBus.emit('time:stop',  time));
        this.timeDriver.on('tick',  (time) => this.eventBus.emit('time:tick',  time));
    }

    static async create(): Promise<ClipEngine>{
        const ctx = await GPUContext.create();
        return new ClipEngine(ctx);
    }

    getContext(): GPUContext{
        return this.ctx;
    }

    getAssetManager(): AssetManager {
        return this.assetManager;
    }

    addTrack(track: BaseTrack){
        this.tracks.push(track);
    }

    getTimeDriver(){ return this.timeDriver; }

    removeTrack(name: string){
        const idx = this.tracks.findIndex(t => t.name === name);
        if(idx >= 0){
            this.tracks[idx].destroy();
            this.tracks.splice(idx, 1);
        }
    }

    render(input: VideoFrame){
        // VideoFrame -> GPUTexture
        const texture = this.ctx.device.createTexture({
            size: [input.displayWidth, input.displayHeight],
            format: this.ctx.format,
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_SRC | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
        });
        
        this.ctx.device.queue.copyExternalImageToTexture(
            { source: input },
            { texture },
            [ input.displayWidth, input.displayHeight ]
        );

        for(let i = 0; i < this.tracks.length; i++){
            this.tracks[i].render(texture);
        }
        texture.destroy();
    }

    on<K extends keyof EngineEvent>(event: K, callback: (payload: EngineEvent[K]) => void) {
        this.eventBus.on(event, callback);
    }

    off<K extends keyof EngineEvent>(event: K, callback: (payload: EngineEvent[K]) => void) {
        this.eventBus.off(event, callback);
    }
}