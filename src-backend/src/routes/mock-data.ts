import fs from 'fs';
import path from 'path';

import { ShaderParamPack, ShaderDescription } from 'clip-engine';

import MosaicParams from '../data/shader/mosaic/params.json';

import CartoonParams from "../data/shader/cartoon/params.json";

import OldFilmParams from "../data/shader/oldfilm/params.json";

import VHSParams from "../data/shader/vhs/params.json";

export function loadShader(relativePath: string): string {
    const fullPath = path.join(process.cwd(), relativePath);
    return fs.readFileSync(fullPath, 'utf-8');
}

export function useMockData(){
    // TODO: 修改为自动查找文件夹下的文件
    const shaders: ShaderDescription[] = [
        {
            name: '马赛克',
            code: loadShader('src/data/shader/mosaic/mosaic.wgsl'),
            params: MosaicParams,
            actived: false
        },
        {
            name: '卡通',
            code: loadShader('src/data/shader/cartoon/cartoon.wgsl'),
            params: CartoonParams,
            actived: false
        },
        {
            name: '老电影',
            code: loadShader('src/data/shader/oldfilm/oldfilm.wgsl'),
            params: OldFilmParams,
            actived: false
        },
        {
            name: 'VHS',
            code: loadShader('src/data/shader/vhs/vhs.wgsl'),
            params: VHSParams,
            actived: false
        }
    ];

    return {
        shaders
    }
}