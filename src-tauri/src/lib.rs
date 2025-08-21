use tauri::{Manager, WebviewWindowBuilder, WindowEvent, LogicalPosition, Position};
use std::sync::Arc;
use tokio::sync::Mutex;
use std::time::Instant;
use tokio::time::{sleep, Duration};

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
            // 获取主窗口
            let main_window = app.get_webview_window("main").unwrap();
            main_window.maximize().unwrap(); // 带顶部栏

            let transparent_window = tauri::WebviewWindowBuilder::new(
                app,
                "transparent",
                tauri::WebviewUrl::App("about:blank".into())
            )
            .title("Transparent Window")
            .transparent(true)
            .decorations(false)
            .always_on_top(false)
            .inner_size(800.0, 600.0)
            .parent(&main_window)
            .unwrap()          // 先 unwrap parent 的 Result
            .build()           // build 返回 Result<WebviewWindow, Error>
            .unwrap();         // 再 unwrap build 的 Result

            // 现在 transparent_window 是 WebviewWindow，可以 clone
            let engine = pollster::block_on(Engine::new(transparent_window.clone()));
            let engine = Arc::new(Mutex::new(engine));
            app.manage(engine.clone());

            let app_handle_clone = app.app_handle().clone();

            // 主窗口移动时让子窗口跟随
            main_window.on_window_event({
                let app_handle = app_handle_clone.clone();
                move |event| {
                    if let WindowEvent::Moved(pos) = event {
                        if let Some(tw) = app_handle.get_webview_window("transparent") {
                            let new_pos = Position::Logical(LogicalPosition {
                                x: pos.x as f64,
                                y: pos.y as f64,
                            });
                            tw.set_position(new_pos).ok();
                        }
                    }
                }
            });

            // 异步渲染循环
            tauri::async_runtime::spawn(async move {
                let engine = app_handle_clone.state::<Arc<Mutex<Engine>>>();
                loop {
                    let t = Instant::now();
                    let mut eng = engine.lock().await;
                    eng.render_frame();
                    println!("Frame rendered in {}ms", t.elapsed().as_millis());
                    sleep(Duration::from_millis(16)).await;
                }
            });

            Ok(())
        })
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![greet])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
