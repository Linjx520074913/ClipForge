/**
 * 资源类型：视频、音频、图片、文本等
 */
pub enum AssetType {
    Video,
    Audio,
    Image,
    Text
}

pub struct Asset {
    id:       String,
    r#type:   AssetType,
    url:      String,
    label:    String,
    duration: f64,          // 仅视频
    width:    Option(u32),  // 仅视频
    height:   Option(u32),  // 仅视频
    cover:    String        // 封面
}