import { TrackRenderer } from './TrackRenderer';
import { GPUContext } from './GPUContext';
import { BaseTrack } from './Track/BaseTrack';
import { TimeDriver } from './Time/TimeDriver';

class TrackGraph{

}

export class ClipEngine{

    private ctx: GPUContext;
    // private trackGraph: TrackGraph;
    private timeDriver: TimeDriver;

    private tracks: BaseTrack[] = [];

    private constructor(ctx: GPUContext){
        this.ctx = ctx;
        this.timeDriver = new TimeDriver(999999999);
    }

    static async create(): Promise<ClipEngine>{
        const ctx = await GPUContext.create();
        return new ClipEngine(ctx);
    }

    getContext(): GPUContext{
        return this.ctx;
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
}