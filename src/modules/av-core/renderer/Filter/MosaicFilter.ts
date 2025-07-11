import IFilter from './Type';
import code from '../shader/mosaic.wgsl?raw';

export default class Mosaic implements IFilter{
    name = 'Mosaic';

    private device!: GPUDevice;
    private format!: GPUTextureFormat;
    private pipeline!: GPURenderPipeline;
    private bindGroup!: GPUBindGroup;
    private sampler!: GPUSampler;
    private inputTexture!: GPUTexture;
    private outputTexture!: GPUTexture;
    private uniformBuffer!: GPUBuffer;

    async init(device: GPUDevice, format: GPUTextureFormat){

        console.error('#######@@@@@@@', typeof code)
        this.device = device;
        this.format = format;

        const module = device.createShaderModule({ code: code });

        this.pipeline = this.device.createRenderPipeline({
            layout: "auto",
            vertex: {
                module,
                entryPoint: "vs_main",
            },
            fragment: {
                module,
                entryPoint: "fs_main",
                targets: [{ format: this.format }],
            },
            primitive: {
                topology: "triangle-list",
            },
        });

        this.sampler = this.device.createSampler({
            magFilter: 'linear',
            minFilter: 'linear'
        });

        // TODO: 这里的 size 要根据参数的数量来设置
        this.uniformBuffer = device.createBuffer({
            size: 8,
            usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
        });
    }
    
    setInput(input: GPUTexture){
        this.inputTexture = input;
        const size = [input.width, input.height];

        const needResize = !this.outputTexture ||
                            this.outputTexture.width != input.width ||
                            this.outputTexture.height!= input.height;

        if(needResize){
            this.outputTexture?.destroy();

            this.outputTexture = this.device.createTexture({
            size,
            format: this.format,
            usage:  GPUTextureUsage.RENDER_ATTACHMENT |
                    GPUTextureUsage.TEXTURE_BINDING |
                    GPUTextureUsage.COPY_SRC
        });
        }
        

        this.bindGroup = this.device.createBindGroup({
            layout: this.pipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: this.sampler },
                { binding: 1, resource: this.inputTexture.createView() },
                { binding: 2, resource: { buffer: this.uniformBuffer } }
            ]
        });

        this.device.queue.writeBuffer(this.uniformBuffer, 0, new Float32Array([input.width, input.height]));
    }

    getOutput(){
        return this.outputTexture;
    }

    updateParams(params: { blockSize: number }) {
        this.device.queue.writeBuffer(this.uniformBuffer, 0, new Float32Array([params.blockSize]));
    }

    render(encoder: GPUCommandEncoder){
        const pass = encoder.beginRenderPass({
            colorAttachments: [
                {
                view: this.outputTexture.createView(),
                loadOp: "clear",
                storeOp: "store",
                clearValue: { r: 0, g: 0, b: 0, a: 1 },
                },
            ],
        });

        pass.setPipeline(this.pipeline);
        pass.setBindGroup(0, this.bindGroup);
        pass.draw(6);
        pass.end();
    }

    destroy(){
        this.uniformBuffer?.destroy();
        this.outputTexture?.destroy();
    }
}