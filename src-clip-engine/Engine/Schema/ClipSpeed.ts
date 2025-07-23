import { z } from 'zod';

export const ClipTrimSchema = z.object({
    value: z.number() // 倍数：1.0 为正常速度
});