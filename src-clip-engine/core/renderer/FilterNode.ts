/**
 * 滤镜管线中的一个处理阶段/节点的意思，和 FilterPipeline 结构匹配度高，也符合视频处理和渲染管线的通用叫法。
 */
export class FilterNode {
    private inputTexture?:  GPUTexture;         // 输入纹理
    private outputTexture?: GPUTexture;        // 输出纹理

    private device?:        GPUDevice;         // GPU 设备
    private format?:        GPUTextureFormat;  // 纹理格式
    private pipeline?:      GPURenderPipeline; // 渲染管线
    private bindGroup!:     GPUBindGroup;
    private sampler!:       GPUSampler;
    private uniformBuffer!: GPUBuffer;
    private bufferSize:     number = 0;

    private shaderCode!: string;              // 着色器代码

    name: string = '';                // 节点名


    constructor(shaderCode: string, name: string = '') {
        this.shaderCode = shaderCode;
        this.name = name;
    }

    /**
     * 初始化
     * @param device
     * @param format
     */
    async init(device: GPUDevice, format: GPUTextureFormat) : Promise<void>{
        
        if (!device) {
            console.error("[ FilterNode ] device is null");
            return;
        }
        if (!format) {
            console.error("[ FilterNode ] format is null");
            return;
        }
        if (!this.shaderCode) {
            console.error("[ FilterNode ] shaderCode is null");
            return;
        }

        this.device = device;
        this.format = format;
        
        const module = this.device.createShaderModule({ code: this.shaderCode });

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
    }

    /**
     * 应用参数
     * @param params 
     */
    applyParams(params: Record<string, number>){
        // if(!this.sampler || !this.device || !this.inputTexture){
        //     console.error("[ FilterNode ] applyParams failed");
        //     return;
        // }

        // const values = Object.values(params);
        // const floatArray = new Float32Array(values);

        // // buffer 大小改变时需要重新创建
        // const needResize = !this.uniformBuffer || this.bufferSize != floatArray.byteLength;

        // if(needResize){
        //     this.uniformBuffer?.destroy();
        //     this.uniformBuffer = this.device?.createBuffer({
        //         size: floatArray.byteLength,
        //         usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
        //     });
        //     this.bufferSize = floatArray.byteLength;
        // }

        // this.device?.queue.writeBuffer(this.uniformBuffer, 0, floatArray);

        // 确保使用最新的 uniformBuffer
        // this.bindGroup = this.device.createBindGroup({
        //     layout: this.pipeline.getBindGroupLayout(0),
        //     entries: [
        //         { binding: 0, resource: this.sampler },
        //         { binding: 1, resource: this.inputTexture.createView() },
        //         // { binding: 2, resource: { buffer: this.uniformBuffer } }
        //     ]
        // });
    }

    /**
     * 设置输入纹理
     * @param texture
     */
    setInputTexture(input: GPUTexture) {
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
    }

    /**
     * 获取输出纹理
     * @returns 输出纹理
     */
    getOutputTexture(): GPUTexture {
        if (!this.outputTexture) {
            throw new Error("[ FilterNode ] outputTexture is null");
        }
        return this.outputTexture;
    }

    /**
     * 渲染
     * @param encoder
     * @returns 
     */
    render(encoder: GPUCommandEncoder){
        // if(!this.bindGroup){
        //     throw new Error(
        //       "[ FilterNode ] bindGroup is null, no params applyed"
        //     );
        // }
        this.bindGroup = this.device.createBindGroup({
            layout: this.pipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: this.sampler },
                { binding: 1, resource: this.inputTexture.createView() },
                // { binding: 2, resource: { buffer: this.uniformBuffer } }
            ]
        });
        if(!this.outputTexture || !this.pipeline || !encoder){
            console.error('[ FilterNode ] render failed');
            return;
        }

        try{
            

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
        }catch(error){
            console.error('[ FilterNode ] render error ', error);
        }
        
    }

    /**
     * 销毁
     */
    destroy(){
        this.uniformBuffer?.destroy();
        this.outputTexture?.destroy();
        this.bindGroup = undefined as any;
    }
}