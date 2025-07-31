#[link(name = "FrameDLL")] // 不要写扩展名
extern "C" {
    fn hello_world() -> *const i8;
}

use std::ffi::CStr;

pub fn call_hello() -> String {
    unsafe {
        let c_str = hello_world();
        CStr::from_ptr(c_str).to_str().unwrap().to_string()
    }
}
