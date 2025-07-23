import { z } from "zod";

export const ClipEffectSchema = z.object({
    id:      z.string(),
    type:    z.string(),
    enabled: z.boolean(),
    order:   z.number()   // 效果应用顺序
});

export const TrackEffectSchema = ClipEffectSchema;