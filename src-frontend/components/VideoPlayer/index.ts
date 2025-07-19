import { ClipEngine, VideoTrack, MP4Player,  } from 'clip-engine';

export function useMP4(){
    const player = new MP4Player();

    return{
        player
    }
}


export function useClipEngine(){
    
    let engine: ClipEngine | null = null;
    let videoTrack: VideoTrack | null = null;

    async function initClipEngine(): Promise<ClipEngine>{
        const engine = await ClipEngine.create();
        return engine;
    }

    return{
        engine,
        videoTrack,
        initClipEngine
    }
}

export {
    VideoTrack
}