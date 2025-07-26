import type { ClipWorkerRequest, ClipWorkerResponse } from './index';

import { MP4Clip } from "@webav/av-cliper";

let clip: MP4Clip | null = null;

async function initClip(url: string): Promise<void> {
    try {
        const resp = await fetch(url);
        clip = new MP4Clip(resp.body!);
        const { duration, width, height } = await clip.ready;
        console.log('Clip init success', width, height, duration);
    } catch(error) {
        throw new Error("Failed to init clip");
    }
}

async function getFrame(time: number): Promise<VideoFrame | null> {
    if(!clip) throw new Error('Clip not initialized');
    const { state, video } = await clip.tick(time * 1000);
    if(state === 'done') return null;
    if(state === 'success' && video) return video;
    return video;
}

self.onmessage = async (e: MessageEvent<ClipWorkerRequest>) => {
    const msg = e.data as ClipWorkerRequest;
    
    try{
        switch(msg.type){
            case 'init':
                {
                    await initClip(msg.url);
                    self.postMessage({ type: 'init-done' });
                }
                break;
            case 'get-frame':
                {
                    const frame = await getFrame(msg.time);
                    if(frame){
                        self.postMessage(
                            { type: 'frame', frame: frame.clone() },
                            [ frame ]
                        );
                        frame.close();
                    }else{
                        self.postMessage(
                            { type: 'frame', frame: null }
                        );
                    }
                }
                break;
            case 'dispose':
                {
                    clip?.destroy();
                    clip = null;
                    self.postMessage({ type: 'dispose-done' });
                }
                break;
            default:
                break;
        }
    }catch(e){
        console.error('Worker error:', e);
    }
};