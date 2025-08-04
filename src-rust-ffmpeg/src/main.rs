mod decoder;
use std::ffi::CStr;

fn main() {
    unsafe {
        let c_str = decoder::get_version();
        print!("{}", CStr::from_ptr(c_str).to_str().unwrap());
    }
}
