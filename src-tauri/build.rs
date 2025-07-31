fn main() {
    println!("cargo:rustc-link-search=native=F:/ClipForge/src-tauri/target/debug");
    tauri_build::build()
}
