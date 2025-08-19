use std::fs;
use std::path::{Path, PathBuf};

fn main() {
    println!("cargo:rustc-link-search=native=target/debug");
    println!("cargo:rustc-link-lib=dylib=cfcore");
    
    let lib_path = PathBuf::from("F:/ClipForge/src-ffmpeg-wrapper/build/Release");
    println!("cargo:rustc-link-search=native={}", lib_path.display());

    let profile = std::env::var("PROFILE").unwrap();
    let target_dir = PathBuf::from(
        std::env::var("CARGO_TARGET_DIR").unwrap_or_else(|_| "target".into())
    )
    .join(&profile);

    copy_recursively(&lib_path, &target_dir);
}

fn copy_recursively(src_dir: &Path, dst_dir: &Path) {
    if let Ok(entries) = fs::read_dir(src_dir) {
        for entry in entries.flatten() {
            let src_path = entry.path();
            let dst_path = dst_dir.join(entry.file_name());

            if src_path.is_dir() {
                // 创建目标目录
                if let Err(e) = fs::create_dir_all(&dst_path) {
                    println!("cargo:warning=Failed to create directory {}: {}", dst_path.display(), e);
                    continue;
                }
                // 递归
                copy_recursively(&src_path, &dst_path);
            } else if src_path.is_file() {
                let filename = src_path.file_name().unwrap().to_string_lossy().to_string();

                let is_wrapper = filename.eq_ignore_ascii_case("ffmpeg_wrapper.dll")
                    || filename.eq_ignore_ascii_case("ffmpeg_wrapper.lib");

                let need_copy = is_wrapper || !dst_path.exists();

                if need_copy {
                    if is_wrapper && dst_path.exists() {
                        // 强制覆盖 wrapper
                        if let Err(e) = fs::remove_file(&dst_path) {
                            println!("cargo:warning=Failed to remove {}: {}", dst_path.display(), e);
                        }
                    }

                    match fs::copy(&src_path, &dst_path) {
                        Ok(_) => println!("cargo:warning=Copied {} -> {}", src_path.display(), dst_path.display()),
                        Err(e) => println!("cargo:warning=Failed to copy {}: {}", src_path.display(), e),
                    }
                }
            }
        }
    } else {
        println!("cargo:warning=Failed to read directory {}", src_dir.display());
    }
}
