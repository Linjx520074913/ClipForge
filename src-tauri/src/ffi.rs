#[repr(C)]
pub struct FFMpegVersion {
    pub major: i32,
    pub minor: i32,
    pub micro: i32
}

#[link(name = "FrameDLL")] // 不要写扩展名
extern "C" {
    pub fn get_ffmpeg_version() -> FFMpegVersion;

    pub fn show_frames(); 
}