import type { IRenderer } from '../renderer/IRenderer';

export type PlayerEvent = 
    | 'ready'       // 播放器准备就绪
    | 'play'        // 播放开始
    | 'pause'       // 播放暂停
    | 'stop'        // 播放停止
    | 'seek'        // 跳转时间
    | 'ended'       // 播放结束
    | 'error'       // 播放错误
    | 'updateFrame' // 更新帧

export interface IPlayer {

    /**
     * 初始化播放器
     * @param renderer 渲染器实例
     */
    initialize(renderer: IRenderer): Promise<void>;

    /**
     * 加载视频
     * @param source 视频源，可以是 URL 字符串或 ReadableStream<Uint8Array>
     */
    load(source: string | ReadableStream<Uint8Array>): Promise<void>;

    /**
     * 播放视频
     */
    play(): void;

    /**
     * 暂停视频
     */
    pause(): void;

    /**
     * 停止播放，并重置进度
     */
    stop(): void;

    /**
     * 跳转到指定时间
     * @param timeUs 指定时间（单位: us）
     */
    seek(timeUs: number): void;

    /**
     * 当前播放位置(单位: us)
     */
    getCurrentTimeUs(): number;

    /**
     * 获取视频总时长(单位: us)
     */
    getDurationUs(): number;

    /**
     * 设置播放速度(1.0 表示正常速度)
     * @param rate 播放速度，取值[1.0, 1.5, 2.0, 2.5, 3.0]
     */
    setPlaybackRate(rate: number): void;

    /**
     * 当前是否正在播放中
     */
    isPlaying(): boolean;

    /**
     * 销毁播放器，释放资源
     */
    destroy(): void;

    /**
     * 注册事件监听
     * @param event 
     * @param callback 回调函数
     */
    on(event: PlayerEvent, callback: () => void): void;

    /**
     * 设置外层容器尺寸，正常要先设置外层容器尺寸，然后在改尺寸下，根据分辨率的比例，
     * 算出最合适的渲染尺寸，及 setOuterSize(outWidth, outHeigth) -> resize(frameWidth, frameHeight)
     */
    setOuterSize(outerWidth: number, outerHeight: number): void;
    
    /**
     * 重置视频大小
     * @param width 
     * @param height 
     */
    resize(frameWidth: number, frameHeight: number): void;
}