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

    private size: { w: number; h: number };
    private frameSize: { w: number; h: number };

    constructor(canvas: HTMLCanvasElement) {
        this.canvas = canvas;
        this.size = { w: 0, h: 0 };
        this.frameSize = { w: 0, h: 0 };
        this.initWebGPU(canvas);
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
        if (this.texture) {
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

    setOuterSize(outerWidth: number, outerHeight: number) {
        console.error("setOuterSize", outerWidth, outerHeight, this.frameSize);
        this.size = { w: outerWidth, h: outerHeight };
        // 如果有 frame 尺寸，重新触发一次渲染
        this.resize(this.frameSize.w, this.frameSize.h, true);
    }

    /**
     * // TODO： 这里的 resize 之后宽或高有可能超出原有的容器宽高
     * 保持渲染的比例与视频帧的比例一直，不会变形
     * @param width
     * @param height
     * @param forceRender 是否强制渲染
     * @returns
     */
    resize(frameWidth: number, frameHeight: number, forceRender: boolean = false) {
       
        if (!this.canvas || frameWidth == 0 || frameHeight == 0) {
            return;
        }

        const frameRadio = frameWidth / frameHeight;
        const canvasRadio = this.canvas.width / this.canvas.height;

        console.error('#######', frameRadio, canvasRadio, forceRender)
        if (frameRadio == canvasRadio && !forceRender) {
            return;
        }

        this.frameSize.w = frameWidth;
        this.frameSize.h = frameHeight;

        let targetW, targetH;
        // 先按宽度算高度
        targetW = this.size.w;
        targetH = this.size.w / frameRadio;

        // 如果高度超出，就用高度算宽度
        if (targetH > this.size.h) {
            targetH = this.size.h;
            targetW = this.size.h * frameRadio;
        }

        console.error("resize", frameWidth, frameHeight, this.frameSize);
        this.canvas.width = frameWidth;
        this.canvas.height = frameHeight;

        this.canvas.style.width = targetW + "px";
        this.canvas.style.height = targetH + "px";
    }

    async render(video: VideoFrame): Promise<void> {
        const renderWidth = video.displayWidth;
        const renderHeight = video.displayHeight;
        this.resize(renderWidth, renderHeight);

        let textureChanged = false;
        if (
            !this.texture ||
            this.texture.width != renderWidth ||
            this.texture.height != renderHeight
        ) {
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
            [renderWidth, renderHeight]
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