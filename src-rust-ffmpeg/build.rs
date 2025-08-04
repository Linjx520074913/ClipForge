use std::fs;
use std::path::PathBuf;

fn main() {
    // 原始 DLL 目录
    let lib_path = PathBuf::from("F:/ClipForge/src-ffmpeg-wrapper/build/Debug");
    println!("cargo:rustc-link-search=native={}", lib_path.display());

    // 目标目录
    let profile = std::env::var("PROFILE").unwrap();
    let target_dir = PathBuf::from(std::env::var("CARGO_TARGET_DIR").unwrap_or_else(|_| "target".into()))
        .join(&profile);

    // 遍历源目录所有文件
    if let Ok(entries) = fs::read_dir(&lib_path) {
        for entry in entries.flatten() {
            let src = entry.path();
            if src.is_file() {
                let dst = target_dir.join(src.file_name().unwrap());
                match fs::copy(&src, &dst) {
                    Ok(_) => println!("cargo:warning=Copied {} -> {}", src.display(), dst.display()),
                    Err(e) => println!("cargo:warning=Failed to copy {}: {}", src.display(), e),
                }
            }
        }
    } else {
        println!("cargo:warning=Failed to read directory {}", lib_path.display());
    }
}
