export class GPUContext{

    public readonly device:  GPUDevice;
    public readonly format:  GPUTextureFormat;
    public readonly sampler: GPUSampler;

    private constructor(device: GPUDevice, format: GPUTextureFormat, sampler: GPUSampler){
        this.device  = device;
        this.format  = format;
        this.sampler = sampler;
    }

    static async create(): Promise<GPUContext>{
        const adapter = await navigator.gpu.requestAdapter({
            powerPreference: "high-performance"
        });
        if(!adapter) throw new Error("WebGPU adapter not found");

        const device = await adapter.requestDevice();
        if(!device) throw new Error("WebGPU device not found");

        const format = navigator.gpu.getPreferredCanvasFormat();
        if(!format) throw new Error("WebGPU preferred canvas format not found");

        const sampler = device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear'
        });
        if(!sampler) throw new Error("WebGPU sampler not found");

        return new GPUContext(device, format, sampler);
    }

}