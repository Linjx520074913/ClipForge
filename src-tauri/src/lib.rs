use tauri::{Manager, WindowEvent, Emitter}; // 添加 WindowEvent 和 Emitter
use serde::Serialize;
use tauri_plugin_dialog::DialogExt;

use std::fs;
use std::path::PathBuf;
use dirs::data_dir;

fn ensure_config_dir() -> PathBuf {
    // 获取 AppData 根目录（Windows: %APPDATA%）
    let mut app_data = data_dir().expect("无法获取 AppData 目录");
    app_data.push("com.clipforge.app");
    app_data.push("ClipForge");

    // 创建目录（如果不存在）
    if !app_data.exists() {
        fs::create_dir_all(&app_data).expect("创建配置目录失败");
    }

    app_data
}

// 定义要发送给前端的窗口位置消息结构体
#[derive(Clone, Serialize)]
struct WindowPositionMessage {
    x: i32,
    y: i32,
    event: String,
}

#[derive(Clone, Serialize)]
struct WindowSizeMessage {
    w: i32,
    h: i32,
    event: String,
}

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[tauri::command]
fn open_file_async(app_handle: tauri::AppHandle) {
    let file_path = app_handle.dialog().file().blocking_pick_file();
    if let Some(path) = file_path {
        println!("open_file_async {:?}", path);
    } else {
        println!("open_file_async cancel");
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let config_dir = ensure_config_dir();
    println!("配置目录: {:?}", config_dir);

    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_fs::init())
        .setup(|app| {
            // 获取主窗口
            let main_window = app.get_webview_window("main").unwrap();
            
            // 克隆 AppHandle 用于在事件回调中使用
            let app_handle = app.app_handle().clone();
            
            main_window.maximize().unwrap(); // 带顶部栏

            // 监听窗口事件
            main_window.on_window_event(move |event| {
                match event {
                    WindowEvent::Moved(position) => {
                        println!("窗口移动到位置: x={}, y={}", position.x, position.y);
                        
                        // 创建要发送的消息
                        let message = WindowPositionMessage {
                            x: position.x,
                            y: position.y,
                            event: "window-moved".to_string(),
                        };
                        
                        // 发送消息到前端
                        if let Err(e) = app_handle.emit("window-event", message) {
                            eprintln!("发送窗口移动事件失败: {}", e);
                        }
                    }
                    WindowEvent::Resized(size) => {
                        println!("窗口调整大小: {}x{}", size.width, size.height);
                        
                        // 也可以发送调整大小的事件
                        let message = WindowSizeMessage {
                            w: size.width as i32,
                            h: size.height as i32,
                            event: "window-resized".to_string(),
                        };
                        
                        if let Err(e) = app_handle.emit("window-event", message) {
                            eprintln!("发送窗口调整大小事件失败: {}", e);
                        }
                    }
                    WindowEvent::Focused(focused) => {
                    }
                    _ => {
                        // 可以处理其他窗口事件
                    }
                }
            });

            Ok(())
        })
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            greet,
            open_file_async
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}