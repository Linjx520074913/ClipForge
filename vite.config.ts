import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import path from "path";
import string from 'vite-plugin-string';

import AutoImport from 'unplugin-auto-import/vite';

// @ts-expect-error process is a nodejs global
const host = process.env.TAURI_DEV_HOST;

// https://vitejs.dev/config/
export default defineConfig(async () => ({
    plugins: [
        vue(),
        string({
            include: ['**/*.wgsl']
        }),
        AutoImport({
            imports: ['vue'],      
            dts: 'src/auto-imports.d.ts',
        })
    ],
    resolve:{
        alias: {
            '@frontend/test':           path.resolve(__dirname, "src-frontend/test"),
            '@frontend/api':            path.resolve(__dirname, "src-frontend/api"),
            '@frontend/assets':         path.resolve(__dirname, "src-frontend/assets"),
            '@frontend/avcore':         path.resolve(__dirname, "src-frontend/modules/av-core"),
            '@frontend/router':         path.resolve(__dirname, "src-frontend/router"),
            '@frontend/store':          path.resolve(__dirname, "src-frontend/store"),
            '@frontend/page':           path.resolve(__dirname, "src-frontend/page"),
            '@frontend/pages':          path.resolve(__dirname, "src-frontend/pages"),
            '@frontend/components':     path.resolve(__dirname, "src-frontend/components"),
            'clip-engine':              path.resolve(__dirname, "src-clip-engine/"),
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
        }
    },
    optimizeDeps: {
        exclude: ['@ffmpeg/ffmpeg']
    },
    assetsInclude: ['**/*.wasm']
}));
