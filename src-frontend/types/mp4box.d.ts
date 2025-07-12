declare module 'mp4box' {
  export interface MP4Sample {
    is_sync: boolean;
    cts: number;
    duration: number;
    data: ArrayBuffer;
  }

  export interface MP4VideoTrack {
    id: number;
    codec: string;
    track_width: number;
    track_height: number;
    nb_samples: number;
  }

  export interface MP4Info {
    videoTracks: MP4VideoTrack[];
  }

  export interface MP4BoxObject {
    write?: (stream: any) => number;
    data?: Uint8Array;
    [key: string]: any;
  }

  export interface MP4File {
    onReady: (info: MP4Info) => void;
    onSamples: (trackId: number, ref: any, samples: MP4Sample[]) => void;
    onError: (err: Error) => void;
    setExtractionOptions: (trackId: number, type: string, options: { nbSamples: number }) => void;
    start: () => void;
    stop: () => void;
    appendBuffer: (buffer: ArrayBuffer) => void;
    flush: () => void;
    moov: {
      traks: Array<{
        mdia: {
          minf: {
            stbl: {
              stsd: {
                entries: Array<{
                  avcC?: Uint8Array | ArrayBuffer | MP4BoxObject;
                  hvcC?: Uint8Array | ArrayBuffer | MP4BoxObject;
                  vpcC?: Uint8Array | ArrayBuffer | MP4BoxObject;
                }>
              }
            }
          }
        }
      }>
    };
  }

  export function createFile(): MP4File;
} 