import { createApp } from "vue";
import App from "./App.vue";
import { createPinia } from "pinia";
import './assets/index.css';
// import 'material-design-icons-iconfont/dist/material-design-icons.css'
// import '@material-design-icons/font';
// TODO:现在 material 用的是 ref link 的方式，需要修改为 CDN 方式

const app = createApp(App)
const pinia = createPinia();

app.use(pinia)
    .mount('#app');
