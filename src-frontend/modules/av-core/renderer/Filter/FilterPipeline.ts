import FilterInst from './FilterInst';

export class FilterPipeline{
    private filters: FilterInst[] = [];
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
    async addFilter(filter: FilterInst){
        await filter.init(this.device, this.format);
        this.filters.push(filter);
    }

    getFilters(): FilterInst[]{
        return this.filters;
    }

    /**
     * 移除滤镜
     * @param name 
     */
    removeFilter(type: string){
        // this.filters = this.filters.filter(f => f.name !== name);
        const idx = this.filters.findIndex(f => f.getFilterType() === type);
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