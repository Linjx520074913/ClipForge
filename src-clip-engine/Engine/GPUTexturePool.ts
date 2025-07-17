import { GPUContext } from "./GPUContext";

/**
 * 纹理池
 */
export class GPUTexturePool{

    private ctx: GPUContext;
    private pool: Map<string, GPUTexture> = new Map();

    constructor(ctx: GPUContext){
        this.ctx = ctx;
    }

    getReusableTexture(width: number, height: number): GPUTexture{
        const key = `${width}x${height}`;

        let texture = this.pool.get(key);
        if(!texture){
            texture = this.ctx.device.createTexture({
                size: [width, height],
                format: this.ctx.format,
                usage: GPUTextureUsage.RENDER_ATTACHMENT | GPUTextureUsage.TEXTURE_BINDING
            });
            this.pool.set(key, texture);
        }

        return texture;
    }

    destroy(){
        for(const texture of this.pool.values()){
            texture.destroy();
        }

        this.pool.clear();
    }

}