import { TrackRenderer } from './renderer/TrackRenderer';

export class ClipEngine{
    
    private device?:  GPUDevice;
    private context?: GPUCanvasContext;
    private format?:  GPUTextureFormat;

    private renderers: TrackRenderer[] = [];

    constructor(){

    }

    /**
     * 
     */
    async init(): Promise<void>{
        await this.initWebGPU();
    }

    /**
     * 初始化 webgpu,必须在组件实例化之后调用
     */
    private async initWebGPU(): Promise<void>{
        try{
            const adapter = await navigator.gpu.requestAdapter({
                powerPreference: "high-performance",
            });
            
            if(!adapter){
                throw new Error("[ ClipEngine ] WebGPU adapter not found");
            }
    
            this.device = await adapter.requestDevice();
            if(!this.device){
                throw new Error("[ ClipEngine ] WebGPU device not found");
            }
    
            this.format = navigator.gpu.getPreferredCanvasFormat();
        }catch(error){
            throw new Error(`[ ClipEngine ] WebGPU init failed: ${error}`);
        }
    }

    addTrackRenderer(renderer: TrackRenderer): void{
        const exists = this.renderers.some(f => f.name === renderer.name);
        if(!exists){
            renderer.init(this.device, this.format);
            this.renderers.push(renderer);
        }
    }

    removeTrackRenderer(name: string): void{
        const idx = this.renderers.findIndex(f => f.name === name);
        if(idx >= 0){
            this.renderers[idx].destroy();
            this.renderers.splice(idx, 1);
        }
    }

    render(){
        for(const renderer of this.renderers){
            renderer.render();
        }
    }
}