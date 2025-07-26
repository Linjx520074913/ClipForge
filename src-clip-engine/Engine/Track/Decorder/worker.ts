import type { ClipWorkerRequest, ClipWorkerResponse } from './ClipDecoder';

import { MP4Clip } from "@webav/av-cliper";

let clip: MP4Clip | null = null;

self.onmessage = async (e: MessageEvent) => {
    const msg = e.data as ClipWorkerRequest;
    
    try{
        switch(msg.type){
            case 'init':
                {
                    const resp = await fetch(msg.url);
                    clip = new MP4Clip(resp.body!);
                    self.postMessage({ type: 'init-done' });
                }
                break;
            case 'get-frame':
                {
                    if(!clip) throw new Error('Clip not initialized');
                    const { state, video } = await clip.tick(msg.time);
                    if (state === 'done') {
                        self.postMessage({ type: 'frame', frame: null });
                        return;
                    }

                    if (state === 'success' && video) {
                        (self).postMessage({ type: 'frame', frame: video.clone() });
                        video.close();
                    }  
                }
                break;
            case 'dispose':
                {
                    clip?.destroy();
                    clip = null;
                }
                break;
            default:
                break;
        }
    }catch(e){
        console.error('Worker error:', e);
    }
};