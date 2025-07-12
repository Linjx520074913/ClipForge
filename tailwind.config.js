/** @type {import('tailwindcss').Config} */
export default {
    content: [
        "./index.html",
        "./src-frontend/**/*.{js,ts,vue,jsx,tsx}",
    ],
    theme: {
        extend: {
            colors: {
            // 主色
            primary: '#F0F0F5',

            // 辅助色：浅蓝绿，hover 或强调用
            accent: '#60A5FA', // blue-400

            // 背景色：超浅灰白，洁净明亮
            'bg-light': '#F0F9FF', // 超浅蓝灰

            // 文字色：深灰，保证可读性
            'text-primary': '#334155', // slate-700

            // 辅助文字色：浅灰，次级文字
            'text-secondary': '#64748B', // slate-500

            // 状态色
            success: '#22C55E', // green-500
            warning: '#FBBF24', // amber-400
            danger: '#EF4444', // red-500

            purple: '#892fff'
        },
        },
    },
    plugins: [],
}

