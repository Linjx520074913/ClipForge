import { BaseTrack } from "./BaseTrack";

/**
 * 轨道渲染器，把统一轨道上的内容渲染到 canvas 上
 */
export class VideoTrack extends BaseTrack{

    type = 'video' as const;
}