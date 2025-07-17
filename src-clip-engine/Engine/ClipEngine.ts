import { TrackRenderer } from './TrackRenderer';
import { GPUContext } from './GPUContext';
import { BaseTrack } from './Track/BaseTrack';

class TrackGraph{

}

export class ClipEngine{

    private ctx: GPUContext;
    // private trackGraph: TrackGraph;

    private tracks: BaseTrack[] = [];

    private constructor(ctx: GPUContext){
        this.ctx = ctx;
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

    render(input: VideoFrame){
        // VideoFrame -> GPUTexture
        const texture = this.ctx.device.createTexture({
            size: [input.displayWidth, input.displayHeight],
            format: 'rgba8unorm',
            usage: GPUTextureUsage.TEXTURE_BINDING | GPUTextureUsage.COPY_DST | GPUTextureUsage.RENDER_ATTACHMENT
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