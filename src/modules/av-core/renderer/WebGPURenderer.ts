import type { IRenderer } from './IRenderer';
import mosaic from './shader/mosaic.wgsl?raw';
import raw from './shader/raw.wgsl?raw';

export class WebGPURenderer implements IRenderer {
    private canvas: HTMLCanvasElement;
    private ctx: GPUCanvasContext | null = null;

    private device: GPUDevice | null = null;

    private texture: GPUTexture | null = null;
    private sampler: GPUSampler | null = null;
    private uniformBuffer: GPUBuffer | null = null;
    private pipeline: GPURenderPipeline | null = null;
    private bindGroup: GPUBindGroup | null = null;
    private format: any;

    private init: boolean = false;

    private size: { w: number, h: number }

    constructor(canvas: HTMLCanvasElement) {
        this.canvas = canvas;
        this.size = { w: canvas.clientWidth, h: canvas.clientHeight }
        this.initWebGPU(canvas)
    }

    private async initWebGPU(canvas: HTMLCanvasElement): Promise<void> {
        const adapter = await navigator.gpu.requestAdapter({
        powerPreference: "high-performance",
        });
        if (!adapter) {
        throw new Error("WebGPU adapter not found");
        }

        this.device = await adapter.requestDevice();
        if (!this.device) {
        throw new Error("WebGPU device not found");
        }

        this.ctx = canvas.getContext("webgpu");
        if (!this.ctx) {
            throw new Error("WebGPU context not found");
        }

        this.format = navigator.gpu.getPreferredCanvasFormat();
        this.ctx.configure({
        device: this.device,
        format: this.format,
        alphaMode: "opaque",
        });
    }

    private async initTexture(width: number, heigth: number) {
        if(this.texture){
            this.texture.destroy();
            this.texture = null;
        }
        if (!this.texture && this.device) {
            this.texture = this.device.createTexture({
                size: [width, heigth, 1],
                format: "rgba8unorm",
                usage:
                GPUTextureUsage.RENDER_ATTACHMENT |
                GPUTextureUsage.COPY_DST |
                GPUTextureUsage.TEXTURE_BINDING,
            });
        }
    }

    private initPipeline() {
        if (!this.device) {
            return;
        }
        if (!this.sampler) {
            this.sampler = this.device.createSampler({
                magFilter: "linear",
                minFilter: "linear",
            });
        }

        if (!this.uniformBuffer) {
            this.uniformBuffer = this.device.createBuffer({
                size: 8, // 2 float: width height
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
            });
        }

        if (!this.pipeline) {
            const module = this.device.createShaderModule({ code: raw });

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

        if (!this.bindGroup || !this.texture) {
            this.bindGroup = this.device.createBindGroup({
                layout: this.pipeline.getBindGroupLayout(0),
                entries: [
                    { binding: 0, resource: this.sampler },
                    { binding: 1, resource: this.texture.createView() },
                    { binding: 2, resource: { buffer: this.uniformBuffer } },
                ],
            });
        }
    }

    /**
     * // TODO： 这里的 resize 之后宽或高有可能超出原有的容器宽高
     * 保持渲染的比例与视频帧的比例一直，不会变形
     * @param width
     * @param height
     * @returns
     */
    resize(width: number, height: number) {
        if (!this.canvas) {
            return;
        }

        const frameRadio = width / height;
        const canvasRadio = this.canvas.width / this.canvas.height;

        if (frameRadio == canvasRadio) {
            return;
        }

        this.canvas.width = width;
        this.canvas.height = height;

        if (frameRadio > canvasRadio) {
            this.canvas.style.width = this.size.h * frameRadio + "px";
            this.canvas.style.height = this.size.h + "px";
            
        } else {
            this.canvas.style.width = this.size.w + "px";
            this.canvas.style.height = this.size.w / frameRadio + "px"; 
        }

        console.error('============', frameRadio, canvasRadio,  this.canvas.style.width, this.canvas.style.height)
    }

    async render(video: VideoFrame): Promise<void> {
        
        const renderWidth = video.displayWidth;
        const renderHeight= video.displayHeight;
        this.resize(renderWidth, renderHeight);

        let textureChanged = false;
        if (
            !this.texture ||
            this.texture.width != renderWidth ||
            this.texture.height != renderHeight
        ){
            await this.initTexture(renderWidth, renderHeight);
            textureChanged = true;
        }

        if (textureChanged || !this.pipeline || !this.bindGroup) {
            this.initPipeline();
        }

        // 拷贝视频帧到当前 GPU 纹理
        const frame = new VideoFrame(video);
        // console.log(video.format, frame.format, video.codedWidth, video.codedHeight, video.displayWidth, video.displayHeight)
        this.device.queue.copyExternalImageToTexture(
            { source: frame },
            { texture: this.texture },
            [ renderWidth, renderHeight ]
        );
        frame.close();

        // 更新 uniform buffer （canvas 大小）
        this.device.queue.writeBuffer(
            this.uniformBuffer,
            0,
            new Float32Array([renderWidth, renderHeight])
        );

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

        this.device.queue.submit([encoder.finish()]);
    }

    destroy() {
        this.uniformBuffer?.destroy?.();
        this.texture?.destroy?.();
        this.sampler = null;
        this.pipeline = null;
        this.bindGroup = null;
        this.ctx = null;
        this.device = null;
    }
}