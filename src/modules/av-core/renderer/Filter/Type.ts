/**
 * 滤镜接口
 */
export interface IFilter{
    name: string;
    /**
     * 初始化管线、shader、bindGroup 等
     * @param device 
     * @param format 
     */
    init(device: GPUDevice, format: GPUTextureFormat): Promise<void>;
    
    /**
     * 设置输入纹理
     * @param input 
     */
    setInput(input: GPUTexture): void;
    
    /**
     * 返回输出纹理（作为下一个滤镜的输入）
     */
    getOutput(): GPUTexture;

    /**
     * 动态更新 shader 参数
     * @param params 
     */
    updateParams(params: any): void;

    /**
     * 执行渲染
     */
    render(encoder: GPUCommandEncoder): void;
    
    /**
     * 销毁资源
     */
    destroy(): void;
}