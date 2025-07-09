import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import path from "path";
import string from 'vite-plugin-string'

// @ts-expect-error process is a nodejs global
const host = process.env.TAURI_DEV_HOST;

// https://vitejs.dev/config/
export default defineConfig(async () => ({
    plugins: [
        vue(),
        string({
            include: ['**/*.wgsl']
        })
    ],
    resolve:{
        alias: {
            '@assets': path.resolve(__dirname, "src/assets"),
            '@avcore':   path.resolve(__dirname, "src/modules/av-core"),
            '@router': path.resolve(__dirname, "src/router"),
            '@store':  path.resolve(__dirname, "src/store"),
            '@src/page':  path.resolve(__dirname, "src/page"),
            '@src/components': path.resolve(__dirname, "src/components")
        }
    },

    // Vite options tailored for Tauri development and only applied in `tauri dev` or `tauri build`
    //
    // 1. prevent vite from obscuring rust errors
    clearScreen: false,
    // 2. tauri expects a fixed port, fail if that port is not available
    server: {
        // port: 1420,
        strictPort: true,
        host: host || false,
        hmr: host
        ? {
            protocol: "ws",
            host,
            // port: 1421,
            }
        : undefined,
        watch: {
        // 3. tell vite to ignore watching `src-tauri`
        ignored: ["**/src-tauri/**"],
        },
    },
}));
