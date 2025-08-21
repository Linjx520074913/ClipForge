fn main() {
    println!("cargo:rustc-link-search=native=D:/ClipForge/src-tauri/target/debug");
    tauri_build::build()
}
