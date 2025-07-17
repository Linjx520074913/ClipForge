import { TrackRenderer } from './TrackRenderer';
import { GPUContext } from './GPUContext';

class TrackGraph{

}

export class ClipEngine{

    private ctx: GPUContext;
    // private trackGraph: TrackGraph;

    private tracks: TrackRenderer[] = [];

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

    addTrack(track: TrackRenderer){
        this.tracks.push(track);
    }

    render(input: VideoFrame){
        for(let i = 0; i < this.tracks.length; i++){
            this.tracks[i].render(input);
        }
    }
}