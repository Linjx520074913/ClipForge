import { WebGPUResourceBase } from "./WebGPUResourceBase";

/**
 * 所有参数统一打包进一个结构体，使用一次 uniformBuffer，绑定到 @binding(n)
 */
type ShaderParamPack = {
    binding: number;   // 对应 WGSL 中 struct 的 binding，比如 @binding(2)
    entries:{
        [paramName: string]:{
            value: number | number[];
            type: 'f32' | 'vec2' | 'vec4';
        };
    };
};

/**
 * 滤镜管线中的一个处理阶段/节点的意思，和 FilterPipeline 结构匹配度高，也符合视频处理和渲染管线的通用叫法。
 */
export class FilterNode extends WebGPUResourceBase {
    private inputTexture?:  GPUTexture;         // 输入纹理
    private outputTexture?: GPUTexture;        // 输出纹理
    private pipeline?:      GPURenderPipeline; // 渲染管线
    private bindGroup?:     GPUBindGroup;
    private sampler?:       GPUSampler;
    private uniformBuffer?: GPUBuffer;
    private bufferSize:     number = 0;
    private shaderCode!: string;              // 着色器代码
    name: string = '';
    private paramsPack?: ShaderParamPack;

    constructor(shaderCode: string, name: string = '', device?: GPUDevice, format?: GPUTextureFormat) {
        super(device, format);
        this.shaderCode = shaderCode;
        this.name = name;
    }

    async init(device: GPUDevice, format: GPUTextureFormat) : Promise<void>{
        this.setDeviceAndFormat(device, format);
        if (!this.device) {
            console.error("[ FilterNode ] device is null");
            return;
        }
        if (!this.format) {
            console.error("[ FilterNode ] format is null");
            return;
        }
        if (!this.shaderCode) {
            console.error("[ FilterNode ] shaderCode is null");
            return;
        }
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

    applyParams(paramPack: ShaderParamPack){
        if(!this.device){
            console.error("[ FilterNode ] applyParams failed: device is null");
            return;
        }
        this.paramsPack = paramPack;
        const { binding, entries } = paramPack;
        const orderedValues: number[] = [];
        for(const key of Object.keys(entries)){
            const { value, type } = entries[key];
            switch(type){
                case 'f32':
                    orderedValues.push(value as number);
                    break;
                case 'vec2':
                    {
                        const arr = value as number[];
                        if(arr.length != 2){
                            console.error("[ FilterNode ] applyParams failed: vec2 type must have 2 values");
                            return;
                        }
                        orderedValues.push(...arr);
                    }
                    break;
                case 'vec4':
                    {
                        const arr = value as number[];
                        if(arr.length != 4){
                            console.error("[ FilterNode ] applyParams failed: vec4 type must have 4 values");
                            return;
                        }
                        orderedValues.push(...arr);
                    }
                    break;
                default:
                    console.error(`[ FilterNode ] applyParams failed: unknown type: ${type}`);
                    break;
            }
        }
        const floatArray = new Float32Array(orderedValues);
        const byteLength = floatArray.byteLength;
        const needResize = !this.uniformBuffer || this.bufferSize != byteLength;
        if(needResize){
            const oldUniformBuffer = this.uniformBuffer;
            this.uniformBuffer = this.device?.createBuffer({
                size: byteLength,
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
            });
            this.bufferSize = byteLength;
            if(oldUniformBuffer) {
                this.delayDestroy(oldUniformBuffer);
            }
        }
        this.device?.queue.writeBuffer(this.uniformBuffer!, 0, floatArray);
    }

    setInputTexture(input: GPUTexture) {
        try{
            this.inputTexture = input;
            const size = [input.width, input.height];
            const needResize = !this.outputTexture ||
                                this.outputTexture.width != input.width ||
                                this.outputTexture.height!= input.height;
            if(needResize){
                const oldOutputTexture = this.outputTexture;
                this.outputTexture = this.device!.createTexture({
                    size,
                    format: this.format!,
                    usage:  GPUTextureUsage.RENDER_ATTACHMENT |
                            GPUTextureUsage.TEXTURE_BINDING |
                            GPUTextureUsage.COPY_SRC
                });
                if(oldOutputTexture) {
                    this.delayDestroy(oldOutputTexture);
                }
            }
        }catch(error){
            console.error(`[ FilterNode ] setInputTexture failed: ${error}`);
        }
    }

    getOutputTexture(): GPUTexture {
        if (!this.outputTexture) {
            throw new Error("[ FilterNode ] outputTexture is null");
        }
        return this.outputTexture;
    }

    render(encoder: GPUCommandEncoder){
        if(!this.outputTexture || !this.pipeline || !encoder){
            console.error('[ FilterNode ] render failed: missing required components');
            return;
        }
        if(!this.paramsPack) {
            console.error('[ FilterNode ] render failed: paramsPack is not initialized');
            return;
        }
        if(!this.uniformBuffer) {
            console.error('[ FilterNode ] render failed: uniformBuffer is not initialized');
            return;
        }
        try{
            if (this.bindGroup) {
                this.bindGroup = undefined;
            }
            this.bindGroup = this.device!.createBindGroup({
                layout: this.pipeline?.getBindGroupLayout(0)!,
                entries:[
                    { binding:0, resource: this.sampler! },
                    { binding:1, resource: this.inputTexture?.createView()! },
                    { binding: this.paramsPack.binding, resource: { buffer: this.uniformBuffer} }
                ]
            });
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
            pass.setPipeline(this.pipeline!);
            pass.setBindGroup(0, this.bindGroup!);
            pass.draw(6);
            pass.end();
        }catch(error){
            console.error(`[ FilterNode ] render failed: ${error}`);
        }
    }

    destroy(){
        console.log(`[ FilterNode ] Destroying ${this.name}...`);
        this.safeDestroy(this.outputTexture);
        this.outputTexture = undefined;
        this.safeDestroy(this.uniformBuffer);
        this.uniformBuffer = undefined;
        this.sampler = undefined;
        this.pipeline = undefined;
        this.bindGroup = undefined;
        this.inputTexture = undefined;
        this.device = undefined;
        this.format = undefined;
        this.paramsPack = undefined;
        console.log(`[ FilterNode ] ${this.name} destroyed`);
    }
}