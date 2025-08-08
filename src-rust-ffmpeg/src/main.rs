use winit::{
    dpi::PhysicalSize,
    event::*,
    event_loop::{ControlFlow, EventLoop},
    window::WindowBuilder,
};

fn main() {
    env_logger::init();
    let event_loop = EventLoop::new();

    // 窗口先隐藏，防止闪烁
    let window = WindowBuilder::new()
        .with_title("Rust Engine Window")
        .with_inner_size(PhysicalSize::new(800, 600))
        .with_visible(false)  // 先隐藏
        .build(&event_loop)
        .unwrap();

    // 稍后显示窗口，尺寸已固定
    window.set_visible(true);

    event_loop.run(move |event, _, control_flow| match event {
        Event::WindowEvent {
            ref event,
            window_id,
        } if window_id == window.id() => match event {
            WindowEvent::CloseRequested
            | WindowEvent::KeyboardInput {
                input:
                    KeyboardInput {
                        state: ElementState::Pressed,
                        virtual_keycode: Some(VirtualKeyCode::Escape),
                        ..
                    },
                ..
            } => *control_flow = ControlFlow::Exit,
            _ => {}
        },
        _ => {}
    });
}
