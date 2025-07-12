import { Filter } from '@src-shared';

export function useMockData(){
    const filter_list: Filter[] = [
        {
            id: 'filter-001',
            type: 'mosaic',
            enabled: false,
            label: '马赛克',
            params: [
                { name: 'pixel_size', label: '像素大小', value: 8, min: 0, max: 2, step: 0.01 }
            ]
        },
        {
            id: 'filter-002',
            type: 'wave',
            enabled: false,
            label: '波浪',
            params: [
                { name: 'u_brightness', value: 1.2, min: 0, max: 2, step: 0.01 }
            ]
        }
    ];
    
    return{
        filter_list
    }
}