import { z } from 'zod';
import { ClipSchema } from './Clip';
import { TrackEffectSchema } from './Effect';

export const TrackTypeEnum = z.enum([ 'video', 'audio', 'image', 'text', 'effect' ]);
export type TrackType = z.infer<typeof TrackTypeEnum>;

/**
 *  id	 : 全局唯一标识符（UUID/雪花ID）。用于内部引用、持久化、拖拽排序、事件映射等。
    name : 用户可读名称，可重复、可修改。用于 UI 展示。
 */
export const TrackSchema = z.object({
    id:   z.string(),
    name: z.string(),
    type: TrackTypeEnum,
    order: z.number(),
    clips: z.array(ClipSchema),
    isLocked: z.boolean(),
    isVisible: z.boolean(),
    isEditing: z.boolean(),
    isMuted: z.boolean(),
    volume: z.number().min(0).max(1),
    opacity: z.number().min(0).max(1),
    blendMode: z.enum(['normal', 'multiply', 'screen', 'overlay']),
    effects: z.array(TrackEffectSchema),
    curClipIndex: z.number()
});