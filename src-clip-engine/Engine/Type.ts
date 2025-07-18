/**
 * 所有参数统一打包进一个结构体，使用一次 uniformBuffer，绑定到 @binding(n)
 */
type ShaderParamPack = {
    binding: number;   // 对应 WGSL 中 struct 的 binding，比如 @binding(2)
    entries:{
        [paramName: string]:{
            value: number | number[];
            type: 'f32' | 'vec2' | 'vec4';
            label: string;
            min: number;
            max: number;
            step: number;
        };
    };
};

type ShaderDescription = {
    name: string;
    code: string;
    params: ShaderParamPack,
    actived: boolean;
}

export type{
    ShaderParamPack,
    ShaderDescription
}