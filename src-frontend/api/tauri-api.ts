import { invoke } from '@tauri-apps/api/core';

export function useTauriApi() {
    async function set_render_window_size(w: number, h: number) {
        console.error('=====set-render-window-size', w, h)
        await invoke('set_render_window_size', { w, h });
    }

    async function set_render_window_position(x: number, y: number) {
        await invoke('set_render_window_position', { x, y });
    }

    return {
        set_render_window_size,
        set_render_window_position
    }
}