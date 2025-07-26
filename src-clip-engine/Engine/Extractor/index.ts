export type ClipWorkerRequest = 
    | { type: 'init';      url: string }
    | { type: 'get-frame'; time: number }
    | { type: 'dispose' }

export type ClipWorkerResponse = 
    | { type: 'init-done' }
    | { type: 'frame'; frame: VideoFrame | null }
    | { type: 'dispose-done' }
    | { type: 'error'; msg: string }