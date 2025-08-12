use std::ffi::{ CString, CStr };
use serde::{Deserialize, Serialize};
use std::os::raw::c_char;
use std::collections::HashMap;

#[derive(Debug, Serialize, Deserialize)]
pub struct AVStreamInfo {
    pub index: i32,
    pub r#type: String,
    pub codec_id: i32,
    pub codec_name: String,
    pub width: i32,
    pub height: i32,
    pub fps: f64,
    pub sample_rate: i32,
    pub entries: HashMap<String, String>
}

#[derive(Debug, Serialize, Deserialize)]
pub struct AVMetadata {
    pub file_path: String,
    pub duration: f64,
    pub bit_rate: i32,
    pub entries: HashMap<String, String>,
    pub streams: Vec<AVStreamInfo>
}

#[link(name="ffmpeg_wrapper", kind="dylib")]
unsafe extern "C" {
    pub fn get_version() -> *const c_char;
    pub fn get_av_meta_data(file_path: *const c_char) ->*const c_char;
    pub fn free_av_meta_data(ptr: *const c_char);
}

pub struct AVDecoder {

}



impl AVDecoder {
    pub fn new() -> Self {
        unsafe {
            
        }

        Self {

        }
    }

    /**
     * 获取版本号
     */
    pub fn get_version() -> String {
        unsafe {
            let version = CStr::from_ptr(get_version()).to_str().unwrap();
            log::info!("get version : {:?}", CStr::from_ptr(get_version()).to_str().unwrap());
            String::from(version)
        }
    }

    /**
     * 获取视频文件的元数据
     */
    pub fn get_av_meta_data(file_path: &str) -> AVMetadata {
        unsafe {
            let c_file_path = CString::new(file_path).expect("CString::new failed");
            let ptr = get_av_meta_data(c_file_path.as_ptr());
            let json_str = CStr::from_ptr(ptr).to_str().expect("Invalid UTF-8");
            let result = serde_json::from_str(json_str).expect("Failed to parse AVMetadata");
            free_av_meta_data(ptr);
            result
        }
    }
}