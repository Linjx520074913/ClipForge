import { FilterPipeline } from "./FilterPipeline";
import RawShaderCode from "./raw.wgsl?raw";
import { WebGPUResourceBase } from "./WebGPUResourceBase";

export class TrackRenderer extends WebGPUResourceBase {

    name: string;

    private ctx?:    GPUCanvasContext;
    private canvas: HTMLCanvasElement;
    private sampler?: GPUSampler;
    private uniformBuffer?: GPUBuffer;
    private bindGroup?: GPUBindGroup;
    private pipeline?: GPURenderPipeline;
    private texture?: GPUTexture;
    private outputTexture?: GPUTexture;
    filterPipeline?: FilterPipeline;
    private rendering: boolean = false;

    constructor(name: string, canvas: HTMLCanvasElement, device?: GPUDevice, format?: GPUTextureFormat) {
        super(device, format);
        this.name = name;
        this.canvas = canvas;
    }

    init(device: GPUDevice, format: GPUTextureFormat){
        this.setDeviceAndFormat(device, format);
        if(!this.canvas || !this.device || !this.format){
            throw new Error(`[ TrackRenderer ] init failed ${this.canvas} ${this.device} ${this.format}`);
        }
        this.ctx = this.canvas.getContext("webgpu");
        this.ctx?.configure({
            device: this.device,
            format: this.format,
            alphaMode: "opaque"
        });
        this.filterPipeline = new FilterPipeline(this.device, this.format);
        this.sampler = this.device.createSampler({
            magFilter: "linear",
            minFilter: "linear",
        });
        this.initPipeline();
    }

    private initPipeline() {
        if (!this.device) {
            return;
        }
        if (!this.uniformBuffer) {
            this.uniformBuffer = this.device.createBuffer({
                size: 8, // 2 float: width height
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
            });
        }
        if (!this.pipeline) {
            const module = this.device.createShaderModule({ code: RawShaderCode });
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
        }
    }

    private initTexture(width: number, height: number) {
        this.safeDestroy(this.texture);
        if (this.device) {
            this.texture = this.device.createTexture({
                size: [width, height, 1],
                format: "rgba8unorm",
                usage:
                GPUTextureUsage.RENDER_ATTACHMENT |
                GPUTextureUsage.COPY_DST |
                GPUTextureUsage.TEXTURE_BINDING,
            });
        }
        this.device?.queue.writeBuffer(
            this.uniformBuffer!,
            0,
            new Float32Array([width, height])
        );
    }

    async render(video: VideoFrame){
        console.error('###########', video)
        if(this.rendering){
            console.warn("[ TrackRenderer ] Skipped render: already rendering");
            return;
        }
        this.rendering = true;
        if(!this.device || !this.ctx || !this.filterPipeline){
            throw new Error("[ TrackRenderer ] render failed");
        }
        try{
            if(video.displayWidth <=0 || video.displayHeight <= 0){
                throw new Error(`[ TrackRenderer ] Invalid video dimensions : ${video.displayWidth} ${video.displayHeight}`);
            }
            const needResize = !this.texture || this.texture.width != video.displayWidth || this.texture.height != video.displayHeight;
            if(needResize){
                await this.initTexture(video.displayWidth, video.displayHeight);
            }
            const frame = new VideoFrame(video);
            try {
                this.device.queue.copyExternalImageToTexture(
                    { source: frame },
                    { texture: this.texture! },
                    [video.displayWidth, video.displayHeight]
                );
            } finally {
                frame.close();
            }
            const oldOutputTexture = this.outputTexture;
            this.outputTexture = await this.filterPipeline.render(this.texture!);
            if(!this.ctx.getCurrentTexture()){
                throw new Error("[ TrackRenderer ]  Failed to get current texture");
            }
            if (this.bindGroup) {
                this.bindGroup = undefined;
            }
            this.bindGroup = this.device.createBindGroup({
                layout: this.pipeline!.getBindGroupLayout(0),
                entries: [
                    { binding: 0, resource: this.sampler! },
                    { binding: 1, resource: this.outputTexture!.createView() },
                    { binding: 2, resource: { buffer: this.uniformBuffer! } },
                ],
            });
            const encoder = this.device.createCommandEncoder();
            const pass = encoder.beginRenderPass({
                colorAttachments: [
                    {
                    view: this.ctx.getCurrentTexture().createView(),
                    loadOp: "clear",
                    storeOp: "store",
                    clearValue: { r: 0, g: 0, b: 0, a: 1 },
                    },
                ],
            });
            pass.setPipeline(this.pipeline!);
            pass.setBindGroup(0, this.bindGroup!);
            pass.draw(6);
            pass.end();
            this.device.queue.submit([encoder.finish()]);
            if(oldOutputTexture && oldOutputTexture !== this.outputTexture) {
                this.delayDestroy(oldOutputTexture);
            }
        }catch(error){
            throw error;
        }finally {
            this.rendering = false;
        }
    }

    destroy() {
        console.log(`[ TrackRenderer ] Destroying ${this.name}...`);
        if (this.filterPipeline) {
            this.filterPipeline.destroy();
            this.filterPipeline = undefined;
        }
        this.safeDestroy(this.outputTexture);
        this.outputTexture = undefined;
        this.safeDestroy(this.texture);
        this.texture = undefined;
        this.safeDestroy(this.uniformBuffer);
        this.uniformBuffer = undefined;
        this.sampler = undefined;
        this.pipeline = undefined;
        this.bindGroup = undefined;
        this.ctx = undefined;
        this.device = undefined;
        this.format = undefined;
        console.log(`[ TrackRenderer ] ${this.name} destroyed`);
    }
}