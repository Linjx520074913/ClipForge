import IFilter from './Filter';

export class FilterPipeline{
    private filters: IFilter[] = [];
    private device:  GPUDevice;
    private format:  GPUTextureFormat;

    private curInput: GPUTexture;

    constructor(device: GPUDevice, format: GPUTextureFormat){
        this.device = device;
        this.format = format;
    }

    /**
     * 添加滤镜
     * @param filter 
     */
    async addFilter(filter: IFilter){
        await filter.init();
        this.filters.push(filter);
    }

    /**
     * 移除滤镜
     * @param name 
     */
    removeFilter(name: string){
        // this.filters = this.filters.filter(f => f.name !== name);
        const idx = this.filters.findIndex(f => f.name === name);
        if(idx >= 0){
            this.filters[idx].destroy();
            this.filters.splice(idx, 1);
        }
    }

    async render(input: GPUTexture): Promise<GPUTexture>{
        if(this.filters.length === 0) return input;

        const encoder = this.device.createCommandEncoder();
        
        this.curInput = input;
        for(const filter of this.filters){
            filter.setInput(this.curInput);
            filter.render(encoder);
            this.curInput = filter.getOutput();
        } 

        this.device.queue.submit([encoder.finish()]);
        return this.curInput;
    }

    destroy(){
        this.filters.forEach((f) => f.destroy());
        this.filters = [];
    }


}