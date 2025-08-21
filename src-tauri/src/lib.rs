// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
use tauri::{ async_runtime, Manager };

use winit::application::ApplicationHandler;
use winit::event::WindowEvent;
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::window::{Window, WindowId};

mod core;
use core::engine::Engine;

use std::sync::Arc;
use std::time::Instant;
use tokio::time::{sleep, Duration};
use tokio::sync::Mutex;

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
            // window.maximize().unwrap();  // 最大化窗口

            let engine = async_runtime::block_on(Engine::new(window));
            let engine = Arc::new(Mutex::new(engine)); // 包装 Mutex
            app.manage(engine.clone());

            let app_handle = app.app_handle().clone();

            async_runtime::spawn(async move {
                let engine = app_handle.state::<Arc<Mutex<Engine>>>();
                loop {
                    let t = Instant::now();

                    {
                        // 获取可变锁
                        let mut eng = engine.lock().await;
                        eng.render_frame(); // ✅ 可以调用 &mut self 方法
                    }

                    println!("Frame rendered in: {}ms", t.elapsed().as_millis());

                    // 避免 CPU 占满
                    sleep(Duration::from_millis(16)).await;
                }
            });
        }
        Ok(())
    })
    .plugin(tauri_plugin_opener::init())
    .invoke_handler(tauri::generate_handler![greet])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
