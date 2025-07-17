import { EffectChain } from "./EffectChain";
import { GPUContext } from "./GPUContext";
import { RendererUnit } from "./RendererUnit";

import RawShaderCode from './RawShader.wgsl?raw';
/**
 * 轨道渲染器，把统一轨道上的内容渲染到 canvas 上
 */
export class TrackRenderer{

    private name: string;

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
        console.error('render', input)
        let output = this.canvasCtx.getCurrentTexture().createView();
        this.mainRenderUnit.process(input, output);
        console.error('FFFFFFFF', input, output)
    }

    destroy(){
        this.effectChain.destroy();
        this.mainRenderUnit.destroy();
    }
}