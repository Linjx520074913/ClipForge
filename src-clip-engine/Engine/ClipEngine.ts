import { GPUContext } from './GPUContext';
import { BaseTrack } from './Track/BaseTrack';
import { TimeDriver } from './Time/TimeDriver';
import { EngineEvent, EventBus } from './EventBus';
import { AssetManager, Asset } from './AssetManager';
import { ClipSchema, ProjectSchema, ClipOption, TrackType, TrackSchema, ShaderSpecSchema } from './Schema';
import { v4 as uuidv4 } from 'uuid';
import { z } from 'zod';
import { ClipFrameExtractor } from './Extractor/ClipFrameExtractor';
import { VideoTrack } from './Track/VideoTrack';

class TrackGraph{

}

type ClipID  = string;
type TrackID = string;
export class ClipEngine {
    private ctx: GPUContext;
    // private trackGraph: TrackGraph;
    private timeDriver: TimeDriver;

    private assetManager: AssetManager;

    private _eventBus: EventBus<EngineEvent>;

    private _project!: ReturnType<typeof ProjectSchema.parse>;

    private _tracks: Map<string, BaseTrack> = new Map();

    private frameExtractors: Map<string, ClipFrameExtractor> = new Map();

    private constructor(ctx: GPUContext) {
        this.ctx = ctx;
        this.timeDriver = new TimeDriver(30000);
        this.assetManager = new AssetManager();
        this._eventBus = new EventBus<EngineEvent>();

        this.timeDriver.on("start", (time) =>
            this._eventBus.emit("time:start", time)
        );
        this.timeDriver.on("pause", (time) =>
            this._eventBus.emit("time:pause", time)
        );
        this.timeDriver.on("stop", (time) => this._eventBus.emit("time:stop", time));
        this.timeDriver.on("tick", async (timeMs: number) => {
            this.render(timeMs);
            this._eventBus.emit("time:tick", timeMs );
        });
    }

    static async create(): Promise<ClipEngine> {
        const ctx = await GPUContext.create();
        return new ClipEngine(ctx);
    }

    getContext(): GPUContext {
        return this.ctx;
    }

    get project() { return this._project; }
    get tracks() { return this._tracks; }

    get eventBus() { return this._eventBus; }

    getAssetManager(): AssetManager {
        return this.assetManager;
    }

    addTrack(track: BaseTrack) {
        // this._tracks.push(track);
    }

    bindClipCanvasToTrack(id: string, canvas: HTMLCanvasElement) {
        
        const track = new VideoTrack(id, this.ctx, canvas);
        this._tracks.set(id, track);
        console.error('bindClipCanvasToTrack', id, this._tracks)
    }

    getTimeDriver() {
        return this.timeDriver;
    }

    removeTrack(name: string) {
        // const idx = this._tracks.findIndex(t => t.name === name);
        // if(idx >= 0){
        //     this._tracks[idx].destroy();
        //     this._tracks.splice(idx, 1);
        // }
    }

    on<K extends keyof EngineEvent>(
        event: K,
        callback: (payload: EngineEvent[K]) => void
    ) {
        this._eventBus.on(event, callback);
    }

    off<K extends keyof EngineEvent>(
        event: K,
        callback: (payload: EngineEvent[K]) => void
    ) {
        this._eventBus.off(event, callback);
    }

    createProject(name: string): void {
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
        const type: TrackType = "video";
        const trackID = `${type}-${this.project.tracks.length}`;
        const trackInput: z.input<typeof TrackSchema> = {
            id: trackID,
            name: trackID,
            type: type,
            order: this.project.tracks.length,
            clips: [],
            isLocked: false,
            isVisible: true,
            isEditing: true,
            isMuted: false,
            volume: 1,
            opacity: 1,
            blendMode: "normal",
            effects: [],
        };

        this._project.tracks.push(trackInput);

        // 添加默认 clip
        const clipID = `${trackID}:clip_0`;
        const clipInput: z.input<typeof ClipSchema> = {
            id: clipID,
            name: clipID,
            type: type,
            assetID: asset.id,
            trackID: trackID,

            isEditing: true,
            isVisible: true,
            isLocked: false,

            startTime: 0,
            duration: asset.duration,

            trim: {
                startTime: 0,
                endTime: 10000,
                offset: 0,
            },
            speed: {
                value: 1.0,
            },
            transformation: {
                position: {
                    x: option.x,
                    y: option.y,
                },
                size: {
                    w: option.width,
                    h: option.height,
                },
                scale: {
                    x: 1,
                    y: 1,
                    uniform: true,
                },
                rotation: 0,
                opacity: 0,
                crop: {
                    left: 0,
                    top: 0,
                    right: 0,
                    bottom: 0,
                },
            },
            effects: []
        };

        this.addClipToTrack(trackID, clipInput);
    }

	addClipToTrack(trackID: string, clip: z.input<typeof ClipSchema>): void {
		const track = this.project.tracks.find((track) => track.id === trackID);
		if (!track) {
		throw new Error("Track not found");
		}
		track.clips.push(clip);

		const extractor = new ClipFrameExtractor();
		// TODO: 根据 assetID 查找对应的 url
		// extractor.initialize(clip.assetID);
		const url = this.assetManager.get(clip.assetID!)?.url;
		if(!url){
			throw new Error(`[ ClipEngine ] : get asset failed ${clip.id}`);
		}
		extractor.initialize(url).then(() => {
            this.timeDriver.seek(0);
        });
		this.frameExtractors.set(clip.id, extractor);
	}

    addEffectToClip(
        trackId: string, 
        clipId: string, 
        effect: z.input<typeof ShaderSpecSchema>): void {
        this._tracks.get(clipId)?.getEffectChain().add(effect);
    }

	render(time: number) {
		this._project.tracks.forEach((track) => {
			track.clips.forEach(async (clip) => {
				clip.isVisible =
				clip.startTime <= time && clip.startTime + clip.duration >= time;
				if (clip.isVisible) {
					const extractor = this.frameExtractors.get(clip.id);
					if (extractor) {
						const frame = await extractor.getFrame(time - clip.startTime);
						
						if (frame && frame.format != null) {
							if (this._tracks.values().size === 0) {
								console.error(
								"[ ClipEngine ] render: not found any track to render"
								);
							}
                            
                            // this._tracks.get(clip.id)?.getEffectChain().add()
							this._tracks.get(clip.id)!.render(frame);
							frame?.close();
						}
					}
				}
			});
		});
	}
}