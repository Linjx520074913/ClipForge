// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
use tauri::{ async_runtime, Manager };

mod core;
use core::engine::Engine;

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
    .setup(|app| {
        #[cfg(debug_assertions)] // 仅在调试构建时包含此代码
        {
            let window = app.get_webview_window("main").unwrap();
            window.maximize().unwrap();  // 最大化窗口

            let engine = async_runtime::block_on(Engine::init(window));
        }
        Ok(())
    })
    .plugin(tauri_plugin_opener::init())
    .invoke_handler(tauri::generate_handler![greet])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
