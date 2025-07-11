// 滤镜类型
export type FilterType = 
    | 'mosaic'
    | 'wave'
    | 'cartoon'

// 每个 Type 映射到一个 Shader 文件或内联字符串
const FilterShaders: Record<FilterType, string> = {
    // brightness: BRIGHTNESS_SHADER,
    // contrast:   BLUR_SHADER
}

// 单个滤镜参数项
export interface FilterParam{
    name:  string;                       // 参数名, 如 brightness
    label: string;                       // 参数文本,如亮度
    value: number | string | boolean;    // 参数值, 如 0.8
    min?:  number;                       // 可选，用于 UI 滑条
    max?:  number;                       // 可选，用于 UI 滑条
    step?: number;
}

// 单个滤镜示例
export interface Filter{
    id:      string;                      // 滤镜 ID
    type:    FilterType;                  // 滤镜类型
    label?:  string;                      // 可显示标签
    enabled: boolean;                     // 是否启用
    params:  FilterParam[];               // 滤镜参数列表
}

/**
 * 应用于某个图层(视频/图片)的滤镜链
 */
export interface FilterChain{
    filters: Filter[];                    // 按顺序执行的滤镜链
}