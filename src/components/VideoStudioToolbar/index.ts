import { ref } from 'vue';

export interface AspecRatioItem{
    label?: string      // 显示文字
    desc?: string       // 文字说明
    icon?: string       // 图标类型
    value?: string      // 绑定值
    divider?: boolean  // 是否是分割线
}

// 视频画面比例配置项数组
const aspectRatios: AspecRatioItem[] = [
    {
        label: '宽 16:9',
        desc: 'YouTube 和流式处理网站',
        icon: 'crop_16_9',
        value: '16:9',
    },
    {
        label: '垂直 9:16',
        desc: 'Instagram Reels 和 TikTok',
        icon: 'crop_portrait',
        value: '9:16',
    },
    {
        label: '正方形 1:1',
        desc: 'Instagram 帖子',
        icon: 'crop_square',
        value: '1:1',
    },
    { 
        divider: true
    },
    {
        label: '经典 4:3',
        icon: 'crop_16_9',
        value: '4:3',
    },
    {
        label: '社交媒体 4:5',
        icon: 'crop_5_4',
        value: '4:5',
    },
    {
        label: '电影 21:9',
        icon: 'crop_7_5',
        value: '21:9',
    },
    {
        label: '纵向 2:3',
        icon: 'crop_portrait',
        value: '2:3',
    },
]

// 下拉菜单是否被激活
const isActived = ref(false);

// 激活的画面配置
const activedRatio = ref(aspectRatios[0]);

const dropdownRef = ref<HTMLElement | null>(null);

export function useAspectRatio(){
    return {
        dropdownRef,
        isActived,
        activedRatio,
        aspectRatios
    }
}
