/*
* 所有参数统一打包进一个结构体，使用一次 uniformBuffer，绑定到 @binding(n)
*/
export type ShaderParamPack = {
    binding: number;                          // 对应 WGSL 中 struct 的 binding，比如 @binding(2)
    entries:{
        [paramName: string]:{
            value: number | number[];
            type: 'f32' | 'vec2' | 'vec4';
        };
    };
};

export interface EffectDefinition{
    name: string;
    shaderCode: string;
    params: ShaderParamPack;
}

export class EffectRegistry{

    static definitions: Map<string, EffectDefinition> = new Map();

    static register(effect: EffectDefinition){
        this.definitions.set(effect.name, effect);
    }

    static get(name: string): EffectDefinition | undefined{
        return this.definitions.get(name);
    }
}