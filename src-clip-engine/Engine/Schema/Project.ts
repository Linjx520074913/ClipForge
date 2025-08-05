import { z } from 'zod';
import { TrackSchema } from './Track';

export const ProjectSettingSchema = z.object({}); // 后续扩展

export const ProjectSchema = z.object({
    id: z.string(),
    name: z.string(),
    setting: ProjectSettingSchema,
    tracks: z.array(TrackSchema),
    curTrackIndex: z.number(),
    createdAt: z.string(),
    updatedAt: z.string()
});