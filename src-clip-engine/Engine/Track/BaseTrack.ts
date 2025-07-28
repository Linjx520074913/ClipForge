import { randomUUID } from "crypto";
import { EffectChain } from "../EffectChain";
import { GPUContext } from "../GPUContext";
import { GPUTexturePool } from "../GPUTexturePool";
import { RendererUnit } from "../RendererUnit";

import RawShaderCode from './RawShader.wgsl?raw';

import { v4 as uuidv4 } from 'uuid';

/**
 *  'video'：视频轨，处理视频片段和转场、滤镜。
    'audio'：音频轨，处理音乐、对话等。
    'image'：静态图像轨，例如照片或海报。
    'text'：文字轨，用于片头片尾、动效字幕等。
    'subtitle'：独立字幕轨（如 SRT），可用于外挂字幕解析。
 */
type TrackType = 'video' | 'audio' | 'image' | 'text' | 'subtitle'

/**
 * 轨道渲染器，把统一轨道上的内容渲染到 canvas 上
 * 轨道基类	BaseTrack	所有轨道继承通用行为
 */
export class BaseTrack{

    private type: TrackType;

    private gputContext: GPUContext;

    private canvas: HTMLCanvasElement;
    private canvasCtx: GPUCanvasContext;
    private effectChain: EffectChain;

    private mainRenderUnit: RendererUnit;

    private texturePool: GPUTexturePool;

    private rendering: boolean = false;

    private id: string;

    constructor(gpuContext: GPUContext, canvas: HTMLCanvasElement){
        this.gputContext = gpuContext;
        this.id = uuidv4();
        this.canvas = canvas;
        this.canvasCtx = this.canvas.getContext('webgpu');

        this.canvasCtx.configure({
            device: gpuContext.device,
            format: gpuContext.format,
            alphaMode: "opaque"
        });

        this.effectChain = new EffectChain(gpuContext);
        
        this.mainRenderUnit = new RendererUnit(gpuContext, "main");
        this.mainRenderUnit.initialize(RawShaderCode);
        this.mainRenderUnit.apply();

        this.texturePool = new GPUTexturePool(gpuContext);
    }

    render(input: GPUTexture | VideoFrame){
        if(!input) throw new Error('[ TrackRenderer ] input is empty');

        if(this.rendering) return;
        
        this.rendering = true;

        const isVideoFrame = input instanceof VideoFrame;

        const w = isVideoFrame? input.displayWidth: input.width;
        const h = isVideoFrame? input.displayHeight:input.height;

        const effectChainOutputTex = this.texturePool.getReusableTexture(w, h);
        if(isVideoFrame){
            const texture = this.texturePool.getReusableTexture(w, h, this.id);
            this.gputContext.device.queue.copyExternalImageToTexture(
                { source: input },
                { texture },
                [ w, h ]
            );
            this.effectChain.process(texture, effectChainOutputTex);
        }else{
            this.effectChain.process(input, effectChainOutputTex);
        }

        this.canvas.width = w;
        this.canvas.height= h;

        // 把 effectChain 输出的纹理渲染到 canvas 上
        this.mainRenderUnit.process(effectChainOutputTex, this.canvasCtx.getCurrentTexture());
        this.rendering = false;
    }

    getEffectChain(){
        return this.effectChain;
    }

    destroy(){
        this.effectChain.destroy();
        this.mainRenderUnit.destroy();
    }
}