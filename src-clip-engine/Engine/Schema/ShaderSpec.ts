import { z } from 'zod';

/**
 * 单个 shader 参数的定义
 */
export const ShaderParamEntry = z.object({
    value: z.union([z.number(), z.array(z.number())]),
    type:  z.enum(['f32', 'vec2', 'vec3', 'vec4']),
    label: z.string(),
    min:   z.number(),
    max:   z.number(),
    step:  z.number(),
});

/**
 * shader 参数统一打包的结构体
 */
export const ShaderParamPack = z.object({
    binding: z.number(),
    entries: z.record(ShaderParamEntry)
});

/**
 * 完整的 shader 规范（包括代码、参数等）
 */
export const ShaderSpecSchema = z.object({
    name:    z.string(),
    code:    z.string(),
    params:  ShaderParamPack,
    enabled: z.boolean()
});

export type ShaderSpec = z.infer<typeof ShaderSpecSchema>;

/**
 * 用法示例
 * import { ShaderSpec } from 'clip-engine';
 * 
 * const shader: ShaderSpec = {
 *  name: 'OldFilm',
 *  code: '...',
 *  enabled: true,
 *  params: {
 *      binding: 2,
 *      entries: {
 *          uNoise: {
 *              value: 0.3,
 *              type: 'f32',
 *              label: '颗粒强度',
 *              min: 0,
 *              max: 1,
 *              step: 0.001,
 *          }
 *      }
 *  }
 * }
 */
