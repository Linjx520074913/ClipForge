import { FilterNode } from './FilterNode';

export class FilterPipeline{
    private filterNodes:     FilterNode[] = [];
    private device:          GPUDevice;
    private format:          GPUTextureFormat;

    private outputTexture?:  GPUTexture;

    constructor(device: GPUDevice, format: GPUTextureFormat){
        this.device = device;
        this.format = format;
    }

    /**
     * 添加滤镜
     * @param node 
     */
    async addFilterNode(node: FilterNode): Promise<void>{
        // 不重复添加相同名字的节点
        const exists = this.filterNodes.some(f => f.name === node.name);
        if(!exists){
            await node.init(this.device, this.format);
            this.filterNodes.push(node);
        }
    }

    /**
     * 
     * @param name 滤镜名
     */
    removeFilterNode(name: string): void{
        const idx = this.filterNodes.findIndex(f => f.name === name);
        if(idx >= 0){
            this.filterNodes[idx].destroy();
            this.filterNodes.splice(idx, 1);
        }
    }

    /**
     * 渲染整条滤镜管线
     * @param input
     * @returns 
     */
    async render(input: GPUTexture): Promise<GPUTexture>{
        this.outputTexture = input;

        if(this.filterNodes.length === 0) return this.outputTexture;

        if (!this.device) throw new Error("GPU device not initialized");

        try{
            const encoder = this.device.createCommandEncoder();
                    
            for(const node of this.filterNodes){
                node.setInputTexture(this.outputTexture);
                node.render(encoder);
                this.outputTexture = node.getOutputTexture();
            }

            this.device.queue.submit([encoder.finish()]);
        }catch(error: any){
            throw new Error("[ FilterPipeline ] : " + error);
        }
        

        return this.outputTexture;
    }

    /**
     * 销毁资源
     */
    destroy(): void{
        this.filterNodes.forEach((f) => f.destroy());
        this.filterNodes = [];
        if (this.outputTexture) {
            this.outputTexture.destroy();
            this.outputTexture = undefined!;
        }
    }
}