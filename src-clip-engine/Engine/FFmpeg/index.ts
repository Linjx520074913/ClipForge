import { createFFmpeg, fetchFile } from '@ffmpeg/ffmpeg';

export class FFmpegEngine{

    private ffmpeg: any;
    
    private constructor(){
        this.ffmpeg = createFFmpeg({ log: true });
    }

    static async create(){
        const inst = new FFmpegEngine();
        if(!inst.ffmpeg.isLoaded()){
            await inst.ffmpeg.load();
        }
    }

    run(command): void{
        await this.ffmpeg.run()
    }
}