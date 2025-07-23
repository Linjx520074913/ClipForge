import { z } from 'zod';

export const ClipTrimSchema = z.object({
    startTime: z.number(), // 源媒体开始时间（单位毫秒）
    endTime:   z.number(), // 源媒体结束时间（单位毫秒）
    offset:    z.number(), // 时间轴上的偏移，单位毫秒
});