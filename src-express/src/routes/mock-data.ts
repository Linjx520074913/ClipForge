import { Filter } from '@src-shared';

export function useMockData(){
    const filter_list: Filter[] = [
        {
            id: 'filter-001',
            type: 'brightness',
            enabled: true,
            label: '黄金时刻',
            params: [
                { name: 'u_brightness', value: 1.2, min: 0, max: 2, step: 0.01 }
            ]
        },
        {
            id: 'filter-002',
            type: 'brightness',
            enabled: true,
            label: '醒目蓝',
            params: [
                { name: 'u_brightness', value: 1.2, min: 0, max: 2, step: 0.01 }
            ]
        },
        {
            id: 'filter-003',
            type: 'brightness',
            enabled: true,
            label: '怀旧西部',
            params: [
                { name: 'u_brightness', value: 1.2, min: 0, max: 2, step: 0.01 }
            ]
        }
    ];
    
    return{
        filter_list
    }
}