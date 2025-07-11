export interface IRenderer {
    
    setFilter(filter: IFilter): void;

    /**
     * 渲染视频帧
     * @param frame 视频帧数据
     */
    render(frame: VideoFrame): void;

    setOuterSize(outerSize: number, outerHeight: number): void;

    /**
     * 重置渲染器大小
     * @param width  目标尺寸宽度
     * @param height 目标尺寸高度
     */
    resize(width: number, height: number): void;

    /**
     * 销毁渲染器，释放资源
     */
    destroy(): void;
}