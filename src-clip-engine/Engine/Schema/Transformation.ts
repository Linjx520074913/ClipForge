import { z } from 'zod';

export const TransformationSchema = z.object({
    position: z.object({
        x: z.number(),                          // 水平位置（像素、百分比）
        y: z.number(),                          // 垂直位置（像素、百分比）
        origin: z.enum(["top-left", "center"])  // 变换原点
    }),
    scale: z.object({
        x: z.number(),                          // 水平缩放（1.0 = 100%）
        y: z.number(),                          // 垂直缩放（1.0 = 100%）
        uniform: z.boolean()                    // 是否保持宽高比
    }),
    rotation: z.number(),                       // 旋转角度（度）
    opacity:  z.number(),                       // 透明度（0.0 = 0%， 1.0 = 100%）
    crop: z.object({
        top:    z.number(),
        bottom: z.number(),
        left:   z.number(),
        right:  z.number()
    })
});