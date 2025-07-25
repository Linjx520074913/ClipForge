import { TrackRenderer } from './TrackRenderer';
import { GPUContext } from './GPUContext';
import { BaseTrack } from './Track/BaseTrack';
import { TimeDriver } from './Time/TimeDriver';
import { EngineEvent, EventBus } from './EventBus';
import { AssetManager, Asset } from './AssetManager';
import { getAsset } from 'node:sea';
import { ClipSchema, ProjectSchema, ClipOption, TrackType, TrackSchema } from './Schema';
import { v4 as uuidv4 } from 'uuid';
import { z } from 'zod';
import { TrackDecoder } from './Track/Decorder';

class TrackGraph{

}

export class ClipEngine{

    private ctx: GPUContext;
    // private trackGraph: TrackGraph;
    private timeDriver: TimeDriver;

    private assetManager: AssetManager;

    private eventBus: EventBus<EngineEvent>;
    
    private _project: ReturnType<typeof ProjectSchema.parse>;

    private tracks: BaseTrack[] = [];

    private trackDecorder: TrackDecoder;

    private constructor(ctx: GPUContext){
        this.ctx = ctx;
        this.timeDriver = new TimeDriver(30000);
        this.assetManager = new AssetManager();
        this.eventBus = new EventBus<EngineEvent>();

        this.trackDecorder = new TrackDecoder('', '');

        this.timeDriver.on('start', (time) => this.eventBus.emit('time:start', time));
        this.timeDriver.on('pause', (time) => this.eventBus.emit('time:pause', time));
        this.timeDriver.on('stop',  (time) => this.eventBus.emit('time:stop',  time));
        this.timeDriver.on('tick',  
            (timeMs: number) =>  {
                this.updateTracks(timeMs);
                this.trackDecorder.getFrame(timeMs);
                this.eventBus.emit('time:tick',  timeMs) 
            }
        );
    }

    static async create(): Promise<ClipEngine>{
        const ctx = await GPUContext.create();
        return new ClipEngine(ctx);
    }

    getContext(): GPUContext{
        return this.ctx;
    }

    get project() { return this._project; }

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

    createProject(name: string): void{
        const input: z.input<typeof ProjectSchema> = {
            id: uuidv4(),
            name: name,
            setting: {},
            tracks: [],
            createdAt: new Date().toISOString(),
            updatedAt: new Date().toISOString(),
        };
        this._project = ProjectSchema.parse(input);
    }

    addVideoTrack(asset: Asset, option: ClipOption): void {
        const type: TrackType = 'video';
        const trackID = `${type}-${this.project.tracks.length}`;
        const trackInput: z.input<typeof TrackSchema> = {
            id:   trackID,
            name: trackID,
            type: type,
            order: this.project.tracks.length,
            clips: [],
            isLocked: false,
            isVisible:true,
            isMuted: false,
            volume: 1,
            opacity: 1,
            blendMode: 'normal',
            effects: []
        };

        this._project.tracks.push(trackInput);

        // 添加默认 clip
        const clipID = `${trackID}:clip_0`;
        const clipInput: z.input<typeof ClipSchema> = {
            id  : clipID,
            name: clipID,
            type: type,
            assetID: asset.id,
            trackID: trackID,

            isEditing: true,
            isVisible: true,
            isLocked : false,
            
            startTime: 0,
            duration : asset.duration,

            trim: {
                startTime: 0,
                endTime  : 10000,
                offset   : 0
            },
            speed: {
                value: 1.0
            },
            transformation: {
                position: {
                    x: option.x,
                    y: option.y
                },
                size: {
                    w: option.width,
                    h: option.height
                },
                scale: {
                    x: 1,
                    y: 1,
                    uniform: true
                },
                rotation: 0,
                opacity:  0,
                crop: {
                    left  : 0,
                    top   : 0,
                    right : 0,
                    bottom: 0
                }
            }
        };

        this.addClipToTrack(trackID, clipInput);
    }

    addClipToTrack(trackID: string, clip: z.input<typeof ClipSchema>): void{
        const track = this.project.tracks.find((track) => track.id === trackID);
        if(!track){
            throw new Error('Track not found');
        }
        track.clips.push(clip);
    }

    /**
     * 根据时间更新轨道状态
     * @param timeMs
     */
    updateTracks(timeMs: number){
        this._project.tracks.forEach((track) => {
            track.clips.forEach((clip) => {
                clip.isVisible = clip.startTime <= timeMs && (clip.startTime + clip.duration) >= timeMs;
            })
        });
    }
}