import { GPUContext } from './GPUContext';
import { RendererUnit } from "./RendererUnit";
import { GPUTexturePool } from "./GPUTexturePool";

export class EffectChain{

    private ctx: GPUContext;
    private units: RendererUnit[] = [];
    private texturePool: GPUTexturePool;

    constructor(ctx: GPUContext){
        this.ctx = ctx;
        this.texturePool = new GPUTexturePool(this.ctx);
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

        let ping = input;
        let pong = this.texturePool.getReusableTexture(input.width, input.height);

        for(let i = 0; i < this.units.length; i++){
            const isLast = i === this.units.length - 1;
            const target = isLast ? output: pong;

            this.units[i].process(ping, target);

            const temp = ping;
            ping = target;
            pong = temp;
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