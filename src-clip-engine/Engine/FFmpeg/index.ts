import { FFmpeg } from '@ffmpeg/ffmpeg';
import { FFprobeWorker } from 'ffprobe-wasm';

export class FFmpegEngine{

    private ffmpeg: FFmpeg;
    
    private constructor(){
        this.ffmpeg = new FFmpeg();
    }

    static async create(): Promise<FFmpegEngine>{
        const inst = new FFmpegEngine();
        if(!inst.ffmpeg.loaded){
            await inst.ffmpeg.load();
        }
        return inst;
    }

    async run(...args: string[]): Promise<void>{
        await this.ffmpeg.run(...args);
    }

    async fetchPartialFile(url: string, length = 65536): Promise<Uint8Array> {
        const res = await fetch(url, {
                headers: {
                Range: `bytes=0-${length - 1}`,
            }
        });
        if (!res.ok) throw new Error(`Range request failed: ${res.status}`);
        const buffer = await res.arrayBuffer();
        return new Uint8Array(buffer);
    }

    async parseMetadataPartial(url: string) {
        const partialData = await this.fetchPartialFile(url);
        const worker = new FFprobeWorker();
        console.log(worker);
        return;
        try {
            const info = await worker.getFileInfo(partialData);
            console.error('FFFFFFFFFFF', info)
            return info;
        } catch (e) {
            console.warn('ffprobe解析失败，可能数据不完整', e);
            return null;
        }
    }

}