import { EffectChain } from "../EffectChain";
import { GPUContext } from "../GPUContext";
import { RendererUnit } from "../RendererUnit";

import RawShaderCode from './RawShader.wgsl?raw';

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

    private name: string;
    private type: TrackType;

    private canvas: HTMLCanvasElement;
    private canvasCtx: GPUCanvasContext;
    private effectChain: EffectChain;

    private mainRenderUnit: RendererUnit;

    constructor(name: string, gpuContext: GPUContext, canvas: HTMLCanvasElement){
        this.name = name;
        this.canvas = canvas;
        this.canvasCtx = this.canvas.getContext('webgpu');

        this.canvasCtx.configure({
            device: gpuContext.device,
            format: gpuContext.format,
            alphaMode: "opaque"
        });

        this.effectChain = new EffectChain(gpuContext);
        
        this.mainRenderUnit = new RendererUnit(gpuContext);
        this.mainRenderUnit.initialize(RawShaderCode);
        this.mainRenderUnit.applyParameters();
    }

    render(input: GPUTexture){
        if(!input) throw new Error('[ TrackRenderer ] input is empty');
        let output = this.canvasCtx.getCurrentTexture();
        this.mainRenderUnit.process(input, output);
    }

    destroy(){
        this.effectChain.destroy();
        this.mainRenderUnit.destroy();
    }
}