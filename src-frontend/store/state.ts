import { computed, ref, watch } from "vue";

import { ConfigStore } from '@frontend/api/config';
const config = new ConfigStore();

const StateRef = ref({
    data: {
        render_wnd_pos:  { x: 0, y: 0 },     // 视频渲染窗位置
        render_wnd_size: { w: 800, h: 600 }, // 视频渲染窗大小
    },
    methods: {
        async init() {
            config.init().then((data) => {
                console.log('用户配置加载完成:', data);
                State.data.render_wnd_pos = data.render_wnd_pos || { x: 0, y: 0 };
                State.data.render_wnd_size = data.render_wnd_size || { w: 800, h: 600 };
            });
        }
    }
});

const State = StateRef.value;
await State.methods.init();

watch(() => State, (value) => {
    // 重新保存配置
    // console.error('保存用户配置:', value)
    if(config) {
        config.set('render_wnd_pos', State.data.render_wnd_pos);
        config.set('render_wnd_size', State.data.render_wnd_size);
    }
}, { deep: true, immediate: true });

export { State }