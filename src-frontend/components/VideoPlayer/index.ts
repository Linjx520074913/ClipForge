import { MP4Clip } from '@webav/av-cliper';
import { ClipEngine, VideoTrack, MP4Player,  } from 'clip-engine';

export function useWebAV(){
    let clip: MP4Clip;
    let videoTrack: VideoTrack;

    function setVideoTrack(track: VideoTrack){
        videoTrack = track;
    }

    async function loadMediaSource(source: string): Promise<void>{
        const resp = await fetch(source);
        clip = new MP4Clip(resp.body);
        const { duration, width, height } = await clip.ready;
        return { duration, width, height };
    }

    async function seek(us: number){
        const { state, video } = await clip.tick(us);
        
        if(state === 'success' && video){
            console.error('seedss', state, video)
            videoTrack.render(video);
            video.close();
        }
    }

    return {
        setVideoTrack,
        loadMediaSource,
        seek
    }
}

export {
    VideoTrack
}