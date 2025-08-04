#[link(name="ffmpeg_wrapper", kind="dylib")]
unsafe extern "C" {
    pub fn get_version() -> *const std::os::raw::c_char;
}