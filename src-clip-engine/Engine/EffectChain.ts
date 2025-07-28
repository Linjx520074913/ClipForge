import { GPUContext } from './GPUContext';
import { RendererUnit } from "./RendererUnit";
import { GPUTexturePool } from "./GPUTexturePool";
import { ShaderDescription } from './Type';

import { ShaderSpecSchema } from './Schema';

import { z } from 'zod';

export class EffectChain{

    private ctx: GPUContext;
    private units: RendererUnit[] = [];
    private texturePool: GPUTexturePool;

    constructor(ctx: GPUContext){
        this.ctx = ctx;
        this.texturePool = new GPUTexturePool(this.ctx);
    }

    /**
     * 添加滤镜节点
     * @param node 
     */
    // addNode(node: RendererUnit): void{
    //     const exist = this.units.some(u => u.name === node.name);
    //     if(!exist){
    //         this.units.push(node);
    //     }
    // }

    addNode(desc: ShaderDescription): void{
       
        const node = new RendererUnit(this.ctx, desc.name);
        node.initialize(desc.code);
        node.applyParameters(desc.params); 
        
        const exist = this.units.some(u => u.name === node.name);
        if(!exist){
            this.units.push(node);
        }
    }

    add(spec: z.input<typeof ShaderSpecSchema>){
        const exist = this.units.some(u => u.name === spec.name);
        if(!exist) {
            const node = new RendererUnit(this.ctx, spec.name);
            node.initialize(spec.code);
            node.apply(spec); 
            
            this.units.push(node);
        }
        // 更新参数
        this.units.filter(u => u.name === spec.name).forEach( n => {
            n.apply(spec);
        });
    }

    /**
     * 移除节点
     * @param name 
     */
    removeNode(name: string): void{
        const idx = this.units.findIndex(u => u.name === name);
        if(idx >= 0){
            this.units[idx].destroy();
            this.units.splice(idx, 1);
        }
    }

    getNode(name: string): RendererUnit | null{
        return this.units.find(u => u.name === name) || null;
    }

    /**
     * 
     * @param input  输入纹理
     * @param output 输出纹理，两者的尺寸一样
     */
    process(input: GPUTexture, output: GPUTexture){

        if (!input || !output) throw new Error(`Missing input/output texture input = ${input}, output = ${output}`);
        if (input.width !== output.width || input.height !== output.height)
        throw new Error("Input and output texture size mismatch");

        try{
            if (this.units.length === 0) {
                const encoder = this.ctx.device.createCommandEncoder();
                encoder.copyTextureToTexture(
                    { texture: input },
                    { texture: output },
                    [input.width, input.height, 1]
                );
                this.ctx.device.queue.submit([encoder.finish()]);
                return;
            }
    
            let ping = input;
            let pong = this.texturePool.getReusableTexture(input.width, input.height);
    
            for(let i = 0; i < this.units.length; i++){
                const isLast = i === this.units.length - 1;
                const target = isLast ? output: pong;
                
                // 如果包含该字段，每帧都更新参数
                // if(this.units[i].params.entries.hasOwnProperty('seed')){
                //     this.units[i].params.entries["seed"].value = Math.random();
                //     this.units[i].applyParameters(this.units[i].params);
                // }
                // this.units[i].apply(this.units[i].p)
                this.units[i].process(ping, target);
    
                const temp = ping;
                ping = target;
                pong = temp;
            }
        }catch(error){
            throw new Error(error);
        }
    }

    destroy(){
        for(let i = 0; i < this.units.length; i++){
            this.units[i].destroy();
        }
        this.units = [];
        this.texturePool.destroy();
    }

}