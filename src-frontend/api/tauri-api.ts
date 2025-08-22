import { invoke } from '@tauri-apps/api/core';
import { open } from '@tauri-apps/plugin-dialog';

export async function set_render_window_size(w: number, h: number) {
    console.error('=====set-render-window-size', w, h)
    await invoke('set_render_window_size', { w, h });
}

export async function set_render_window_position(x: number, y: number) {
    await invoke('set_render_window_position', { x, y });
}

export async function open_file_async() {
    await invoke('open_file_async');
}