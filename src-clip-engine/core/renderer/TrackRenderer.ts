import { FilterPipeline } from "./FilterPipeline";
import RawShaderCode from "./raw.wgsl?raw";

export class TrackRenderer {

    name: string;

    private device?: GPUDevice;
    private format?: GPUTextureFormat;
    private ctx?:    GPUCanvasContext;

    private canvas: HTMLCanvasElement;
    private sampler: GPUSampler;
    private uniformBuffer: GPUBuffer;
    private bindGroup: GPUBindGroup;
    private pipeline: GPURenderPipeline;

    private texture: GPUTexture;
    private outputTexture: GPUTexture;

    filterPipeline?: FilterPipeline;

    private rendering: boolean = false;

    constructor(name: string, canvas: HTMLCanvasElement) {
        this.name = name;
        this.canvas = canvas;
    }

    init(device: GPUDevice, format: GPUTextureFormat){
        this.device = device;
        this.format = format;

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
        if (this.texture) {
            this.texture.destroy();
            this.texture = null;
        }
        if (!this.texture && this.device) {
            this.texture = this.device.createTexture({
                size: [width, height, 1],
                format: "rgba8unorm",
                usage:
                GPUTextureUsage.RENDER_ATTACHMENT |
                GPUTextureUsage.COPY_DST |
                GPUTextureUsage.TEXTURE_BINDING,
            });
        }

        this.device.queue.writeBuffer(
            this.uniformBuffer,
            0,
            new Float32Array([width, height])
        );
    }

    async render(video: VideoFrame){
        if(this.rendering){
            console.warn("[ TrackRenderer ] Skipped render: already rendering");
            return;
        }

        this.rendering = true;

        if(!this.device || !this.ctx || !this.filterPipeline){
            throw new Error("[ TrackRenderer ] render failed");
        }

        try{
            // 检查视频尺寸
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

            // 保存旧的输出纹理引用，稍后销毁
            const oldOutputTexture = this.outputTexture;

            // 经过滤镜管线处理，得到最终输出纹理
            this.outputTexture = await this.filterPipeline.render(this.texture!);

            if(!this.ctx.getCurrentTexture()){
                throw new Error("[ TrackRenderer ]  Failed to get current texture");
            }

            this.bindGroup = this.device.createBindGroup({
                layout: this.pipeline.getBindGroupLayout(0),
                entries: [
                    { binding: 0, resource: this.sampler },
                    { binding: 1, resource: this.outputTexture.createView() },
                    { binding: 2, resource: { buffer: this.uniformBuffer } },
                ],
            });

            // 开始渲染
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

            pass.setPipeline(this.pipeline);
            pass.setBindGroup(0, this.bindGroup);
            pass.draw(6);
            pass.end();

            // 提交命令缓冲区
            this.device.queue.submit([encoder.finish()]);

            // 使用onSubmittedWorkDone确保命令完成后再销毁旧纹理
            if(oldOutputTexture && oldOutputTexture !== this.outputTexture) {
                this.device.queue.onSubmittedWorkDone().then(() => {
                    oldOutputTexture.destroy();
                });
            }
        }catch(error){
            throw error; // 或者根
        }finally{
            this.bindGroup = null;
            this.rendering = false;
        }
    }

    /**
     * 销毁
     */
    destroy() {
        this.uniformBuffer?.destroy?.();
        this.sampler = null;
        this.pipeline = null;
        this.bindGroup = null;
        this.ctx = null;
        this.device = null;
        this.filterPipeline?.destroy();
        this.outputTexture?.destroy();
        this.texture?.destroy?.();
    }
}