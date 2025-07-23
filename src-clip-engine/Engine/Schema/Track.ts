import { z } from 'zod';
import { ClipSchema } from './Clip';
import { TrackEffectSchema } from './Effect';

export const TrackSchema = z.object({
    id:   z.string(),
    name: z.string(),
    type: z.enum([ 'video', 'audio', 'image', 'text', 'effect' ]),
    order: z.number(),
    clips: z.array(ClipSchema),
    isLocked: z.boolean(),
    isVisivle: z.boolean(),
    isMuted: z.boolean(),
    volume: z.number().min(0).max(1),
    opacity: z.number().min(0).max(1),
    blendMode: z.enum(['normal', 'multiply', 'screen', 'overlay']),
    effects: z.array(TrackEffectSchema)
});