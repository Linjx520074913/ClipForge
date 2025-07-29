export interface ClipWorkerRequest {
    type : 'init' | 'get-frame' | 'dispose';
    url? : string;
    time?: number;
    requestId?: string;
}

export interface ClipWorkerResponse {
    type: 'init-done' | 'frame' | 'dispose-done' | 'error';
    frame?: VideoFrame | null;
    msg?: string;
    time?: number;
    requestId?: string;
}