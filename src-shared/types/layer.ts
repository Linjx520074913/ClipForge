interface IMediaSource {
    id:     string;                                  // 唯一标识符
    type:   "video" | "audio" | "image" | "text";    // 视频、音频、图片、文本
    uri:    string;                                  // 本地路径,网络 URL，Blob URL
    text?:  string;                                  // 当 type == text 时该字段有效
    width:  number;
    height :number;                                  // 当 type == video 或 image 时该字段有效
}

interface ILayer{
    id:     string;
    type:   'video' | 'audio' | 'image' | 'text';
    label?: string;                                  // 图层标签  
    source: IMediaSource;
    zIndex: number;
    active: boolean;
    size: {                                          // 图层大小
        w: number,
        h: number,
    },
    pos: {                                           // 图层位置
        x: number,
        y: number
    }
}

export { ILayer }