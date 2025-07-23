import { z } from 'zod';
import { ClipSchema } from './Clip';
import { TrackEffectSchema } from './Effect';

export const TrackTypeEnum = z.enum([ 'video', 'audio', 'image', 'text', 'effect' ]);
export type TrackType = z.infer<typeof TrackTypeEnum>;

export const TrackSchema = z.object({
    id:   z.string(),
    name: z.string(),
    type: TrackTypeEnum,
    order: z.number(),
    clips: z.array(ClipSchema),
    isLocked: z.boolean(),
    isVisible: z.boolean(),
    isMuted: z.boolean(),
    volume: z.number().min(0).max(1),
    opacity: z.number().min(0).max(1),
    blendMode: z.enum(['normal', 'multiply', 'screen', 'overlay']),
    effects: z.array(TrackEffectSchema)
});