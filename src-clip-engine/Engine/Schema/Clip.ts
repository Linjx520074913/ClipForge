import { z } from 'zod';
import { ClipTrimSchema } from './ClipTrim';
import { ClipSpeedSchema } from "./ClipSpeed";
import { TransformationSchema } from './Transformation';
import { ClipEffectSchema } from './Effect';

export const ClipSchema = z.object({
    id: z.string(),
    name: z.string(),
    type: z.enum(['video', 'audio', 'image', 'text', 'transition']),
    assedId: z.string().nullable(),
    trackId: z.string(),

    // 时间属性
    startTime: z.number(),     // 时间轴上的开始时间（毫秒）
    duration:  z.number(),     // 总时长（毫秒）
    trim:  ClipTrimSchema,
    speed: ClipSpeedSchema,

    // 空间属性
    transformation: TransformationSchema,
    
    // 内容属性
    effects: z.array(ClipEffectSchema)
});