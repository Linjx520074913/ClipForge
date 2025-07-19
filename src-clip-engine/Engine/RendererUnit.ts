import { GPUContext } from './GPUContext';
import { ShaderParamPack } from './Type';

/**
 * 渲染单元，负责把输入纹理处理后输出
 */
export class RendererUnit{

    private name: string;

    private ctx: GPUContext;

    private pipeline:      GPURenderPipeline;
    private bindGroup:     GPUBindGroup;
    private uniformBuffer: GPUBuffer;

    constructor(ctx: GPUContext, name: string = ""){
        this.ctx = ctx;
        this.name = name;
    }

    /**
     * 初始化管线，shaderCode 是包含 vs_main 和 fs_main 的 WGSL 代码
     * @param shaderCode 
     */
    initialize(shaderCode: string){

        const module = this.ctx.device.createShaderModule({ code: shaderCode });

        this.pipeline = this.ctx.device.createRenderPipeline({
            layout: "auto",
            vertex: {
                module,
                entryPoint: "vs_main",
            },
            fragment: {
                module,
                entryPoint: "fs_main",
                targets: [{ format: this.ctx.format }],
            },
            primitive: {
                topology: "triangle-list",
            }
        });
    }

    /**
     * 应用 shader 参数
     * @param param 
     * @returns 
     */
    applyParameters(param: ShaderParamPack | undefined){
        console.error('+============= applyParams', param)
        if(!param) return;
        
        const { binding, entries } = param;
        const uniformValues: number[] = [];
        
        // 解包 param，然后把 entries 中的参数放入到 uniformValues 中
        for(const key of Object.keys(entries)){
            const { value, type } = entries[key];

            switch(type){
                case 'f32':
                    {
                        uniformValues.push(value as number);
                    }
                break;
                case 'vec2':
                    {
                        const arr = value as number[];
                        if(arr.length != 2){
                            console.error(`[ RendererUnit ] vec2 must have 2 elements : ${value}`);
                            return;
                        }else{
                            uniformValues.push(...arr)
                        }
                    }
                break;
                case 'vec4':
                    {
                        const arr = value as number[];
                        if(arr.length != 4){
                            console.error(`[ RendererUnit ] vec4 must have 4 elements : ${value}`);
                        }else{
                            uniformValues.push(...arr);
                        }
                    }
                break;
                default:
                    console.error(`[ RendererUnit ] unknown type: ${type}`);
                    break;
            }
        }

        const floatArray = new Float32Array(uniformValues)
        if(!this.uniformBuffer){
            this.uniformBuffer = this.ctx.device.createBuffer({
                size: floatArray.byteLength,
                usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
            })
        }

        this.ctx.device.queue.writeBuffer(this.uniformBuffer, 0, floatArray);
    }

    /**
     * 检查异常
     */
    private checkException(){
        if(!this.pipeline){
            console.error("[ RendererUnit ] pipeline not initialized!");
        }
    }

    /**
     * 处理：将 input 纹理经过处理输出到 output 纹理上
     * @param input 
     * @param output 
     */
    process(input: GPUTexture, output: GPUTexture){
        this.checkException();
        try{
            const entries: GPUBindGroupEntry[] = [
                { binding: 0, resource: this.ctx.sampler },
                { binding: 1, resource: input.createView() },
            ];
            if(this.uniformBuffer){
                entries.push({ binding: 2, resource: { buffer: this.uniformBuffer }});
            }

            this.bindGroup = this.ctx.device.createBindGroup({
                layout: this.pipeline.getBindGroupLayout(0),
                entries
            });

            /**
             * 每帧渲染流程
            -> create commandEncoder
                -> beginRenderPass
                    -> setPipeline setBindGroup draw
                -> endPass
            -> submit to GPU queue
            */
            const commandEncoder = this.ctx.device.createCommandEncoder();
            const renderPass = commandEncoder.beginRenderPass({
                colorAttachments: [
                    {
                        view: output.createView(),
                        loadOp:  "clear",
                        storeOp: "store",
                        clearValue: { r: 0, g: 0, b: 0, a: 1 },
                    }
                ]
            });
            renderPass.setPipeline(this.pipeline);
            renderPass.setBindGroup(0, this.bindGroup);
            renderPass.draw(6, 1, 0, 0);
            renderPass.end();

            this.ctx.device.queue.submit([commandEncoder.finish()]);
        }catch(error){
            console.error('[ RendererUnit ] ', error)
        }
        
    }

    destroy(){
        this.uniformBuffer?.destroy?.();
        this.pipeline = null;
        this.bindGroup = null;
        this.uniformBuffer = null;
    }

}