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
import { FFmpegEngine } from './FFmpeg';
import { EffectChain } from './EffectChain';

class TrackGraph{

}

/**
 * Track 和 Clip 的命名规则
 * Track 根据类型命名，Clip 名字继承 asset 名字
 
Track: "Video Track 1"
 └── Clip: "Intro.mp4"
 └── Clip: "Scene 2 Video Clip"

Track: "Audio Track 1"
 └── Clip: "Background Music"

**/

interface TrackRuntime {
    id: string;
    clipRuntime: Map<string, ClipRuntime>;
}

interface ClipRuntime {
    id: string;
    renderer: BaseTrack;
    extractor: ClipFrameExtractor;
}

export class ClipEngine {
    private ctx: GPUContext;

    private timeDriver: TimeDriver;

    private assetManager: AssetManager;

    private _eventBus: EventBus<EngineEvent>;

    private _project!: ReturnType<typeof ProjectSchema.parse>;

    private trackRuntime: Map<string, TrackRuntime> = new Map();

    private _ffmpeg: FFmpegEngine;

    private rendering: boolean = false;

    private constructor(ctx: GPUContext, ffmpeg: FFmpegEngine) {
        this.ctx = ctx;
        this.timeDriver = new TimeDriver(30000);
        this.assetManager = new AssetManager();
        this._eventBus = new EventBus<EngineEvent>();
        this._ffmpeg = ffmpeg;

        this.timeDriver.on("start", (time) =>
        this._eventBus.emit("time:start", time)
        );
        this.timeDriver.on("pause", (time) =>
        this._eventBus.emit("time:pause", time)
        );
        this.timeDriver.on("stop", (time) =>
        this._eventBus.emit("time:stop", time)
        );
        this.timeDriver.on("tick", async (timeMs: number) => {
            this.render(timeMs);
            this._eventBus.emit("time:tick", timeMs);
        });
    }

    static async create(): Promise<ClipEngine> {
        const ctx = await GPUContext.create();
        const ffmpeg = await FFmpegEngine.create();
        return new ClipEngine(ctx, ffmpeg);
    }

    getContext(): GPUContext {
        return this.ctx;
    }

    get project() {
        return this._project;
    }
    get tracks() {
        return this._clipRenderer;
    }
    get eventBus() {
        return this._eventBus;
    }
    get ffmpeg() {
        return this._ffmpeg;
    }

    getAssetManager(): AssetManager {
        return this.assetManager;
    }

    addTrack(track: BaseTrack) {
        // this._clipRenderer.push(track);
    }

    bindClipCanvasToTrack(
        trackId: string,
        clipId: string,
        canvas: HTMLCanvasElement
    ) {
        const renderer = new VideoTrack(this.ctx, canvas);

        if (!this.trackRuntime.get(trackId)) {
        this.trackRuntime.set(trackId, new Map<string, ClipRuntime>());
        }

        if (!this.trackRuntime.get(trackId).get(clipId)) {
        this.trackRuntime.get(trackId).set(clipId, {
            id: clip.id,
        });
        }

        this.trackRuntime.get(trackId).get(clipId)!.renderer = renderer;
    }

    getTimeDriver() {
        return this.timeDriver;
    }

    removeTrack(name: string) {
        // const idx = this._clipRenderer.findIndex(t => t.name === name);
        // if(idx >= 0){
        //     this._clipRenderer[idx].destroy();
        //     this._clipRenderer.splice(idx, 1);
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
            curTrackIndex: 0,
            createdAt: new Date().toISOString(),
            updatedAt: new Date().toISOString(),
        };
        this._project = ProjectSchema.parse(input);
    }

    addVideoTrack(asset: Asset, option: ClipOption): void {
        const type: TrackType = "video";
        const trackId = uuidv4();
        const trackInput: z.input<typeof TrackSchema> = {
        id: trackId,
        name: `Video Track ${this.project.tracks.length}`,
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
        curClipIndex: 0,
        };

        this._project.tracks.push(trackInput);
        this._project.curTrackIndex = this._project.tracks.length - 1;

        // 添加默认 clip
        const clipId = uuidv4();
        const clipInput: z.input<typeof ClipSchema> = {
        id: clipId,
        name: asset.id,
        type: type,
        assetId: asset.id,
        trackId: trackId,

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
        effects: [],
        curEffectIndex: 0,
        };

        this.addClipToTrack(trackId, clipInput);
    }

    addClipToTrack(trackId: string, clip: z.input<typeof ClipSchema>): void {
        const track = this._project.tracks.find((track) => track.id === trackId);
        if (!track) {
        throw new Error("Track not found");
        }
        track.clips.push(clip);

        const extractor = new ClipFrameExtractor();
        // TODO: 根据 assetID 查找对应的 url
        // extractor.initialize(clip.assetID);
        const url = this.assetManager.get(clip.assetId!)?.url;
        if (!url) {
        throw new Error(`[ ClipEngine ] : get asset failed ${clip.id}`);
        }
        extractor.initialize(url).then(async (id) => {
            // this.timeDriver.seek(0);
            // 渲染第一帧
            
            console.error('+++++FFFFFFFFFFFFFFFF')
            const frame = await extractor.getFrame(0);
            console.error('#@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@', frame)
            this.trackRuntime.get(trackId).get(clip.id).renderer.render(frame);
        });

        if (!this.trackRuntime.get(trackId)) {
        this.trackRuntime.set(trackId, new Map<string, ClipRuntime>());
        }

        if (!this.trackRuntime.get(trackId).get(clip.id)) {
        this.trackRuntime.get(trackId).set(clip.id, {
            id: clip.id,
        });
        }

        this.trackRuntime.get(trackId).get(clip.id)!.extractor = extractor;
    }

    render(time: number | undefined) {
        time = time || this.timeDriver.curTimeMs;
        
        if(this.rendering) return;
        this.rendering = true;
        this._project.tracks.forEach((track) => {
            if(!track.isVisible) return;

            track.clips.forEach(async (clip) => {
                clip.isVisible = clip.startTime <= time && clip.startTime + clip.duration >= time;
                if (clip.isVisible) {
                    if (this.trackRuntime.size == 0) {
                        console.error(
                        "[ ClipEngine ] : trackRuntime is empty, please invoke addVideoTrack or addAudioTrack before render"
                        );
                        return;
                    }
                    const trackRuntime: TrackRuntime = this.trackRuntime.get(track.id);
                    if (!trackRuntime) {
                        console.error(
                        `[ ClipEngine ] : cant not get trackRuntime for track ${track.id}`
                        );
                        return;
                    }

                    const clipRuntime: ClipRuntime = trackRuntime.get(clip.id);
                    if (!clipRuntime) {
                        console.error(
                        `[ ClipEngine ] cant not get clipRuntime for clip ${clip.id}`
                        );
                        return;
                    }

                    const renderer: BaseTrack = clipRuntime.renderer;
                    const extractor: ClipFrameExtractor = clipRuntime.extractor;
                    if (renderer && extractor) {
                        const frame = await extractor.getFrame(time - clip.startTime);
                        // extractor.preload(time - clip.startTime);
                        if (frame && frame.format != null) {
                            const effectChain: EffectChain = renderer.getEffectChain();
                            clip.effects.forEach((effect) => {
                                effectChain.add(effect);
                            });
                            renderer.render(frame);
                        }
                    }
                }
            });
        });
        this.rendering = false;
    }
}