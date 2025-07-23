type EventCallback<T = any> = (payload: T) => void;

export type EngineEvent = {
    // ClipEngine 本身生命周期事件
    'engine:ready': void;
    'engine:error': { message: string };

    // 时间驱动相关
    'time:start': { curTimeMs: number };
    'time:pause': { curTimeMs: number };
    'time:stop':  { curTimeMs: number };
    'time:seek':  { curTimeMs: number };
    'time:ended': void;
    'time:tick':  { curTimeMs: number };
};

export class EventBus<Events extends Record<string, any>>{
    private listeners: {[K in keyof Events]? : EventCallback<Events[K]>[]} = {};

    on<K extends keyof Events>(event: K, callback: EventCallback<Events[K]>){
        if(!this.listeners[event]) this.listeners[event] = [];
        this.listeners[event]!.push(callback);
    }

    off<K extends keyof Events>(event: K, callback: EventCallback<Events[K]>){
        this.listeners[event] = this.listeners[event]?.filter(h => h !== callback);
    }

    emit<K extends keyof Events>(event: K, payload: Events[K]) {
        this.listeners[event]?.forEach(c => c(payload));
    }

    clear(){
        this.listeners = {};
    }
}