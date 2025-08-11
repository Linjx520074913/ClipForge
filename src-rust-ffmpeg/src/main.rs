use winit::application::ApplicationHandler;
use winit::event::WindowEvent;
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::window::{Window, WindowId};

mod core;
use core::engine::Engine;

#[derive(Default)]
struct App {
    window: Option<Window>,
    engine: Option<Engine>

}

impl ApplicationHandler for App {
    /**
     * 应用恢复，创建窗口
     */
    fn resumed(&mut self, event_loop: &ActiveEventLoop) {
        let window = event_loop.create_window(Window::default_attributes()).unwrap();
        
        let engine = pollster::block_on(Engine::new(&window));

        self.window = Some(window);
        self.engine = Some(engine);
    }

    /**
     * 监听窗口事件
     */
    fn window_event(&mut self, event_loop: &ActiveEventLoop, id: WindowId, event: WindowEvent) {
        match event {
            WindowEvent::CloseRequested => {
                println!("The close button was pressed; stopping");
                event_loop.exit();
            },
            WindowEvent::RedrawRequested => {
                
                // 调用 WGPU 渲染逻辑

                // Redraw the application.
                //
                // It's preferable for applications that do not render continuously to render in
                // this event rather than in AboutToWait, since rendering in here allows
                // the program to gracefully handle redraws requested by the OS.

                // Draw.

                // Queue a RedrawRequested event.
                //
                // You only need to call this if you've determined that you need to redraw in
                // applications which do not always need to. Applications that redraw continuously
                // can render here instead.
                self.window.as_ref().unwrap().request_redraw();
            }
            _ => (),
        }
    }
}

fn init_env_logger() {
    env_logger::Builder::new()
        .filter_level(log::LevelFilter::Info)
        .init();
    log::info!("Starting application : init_env_logger done");
}

fn main() {
    init_env_logger();

    let event_loop = EventLoop::new().unwrap();

    // ControlFlow::Poll continuously runs the event loop, even if the OS hasn't
    // dispatched any events. This is ideal for games and similar applications.
    event_loop.set_control_flow(ControlFlow::Poll);

    let mut app = App::default();
    event_loop.run_app(&mut app).expect("Failed to run app");
}