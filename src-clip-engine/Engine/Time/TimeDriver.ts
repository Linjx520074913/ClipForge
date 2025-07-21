/**
 * 时间驱动的主要职责
 *  1. 播放时间推进	基于 performance.now() 与 requestAnimationFrame，计算每帧的 deltaTime 并更新 currentTime
    2. 播放控制状态管理	控制播放（start）、暂停（pause）、停止（stop）、跳转（seek）状态切换与逻辑正确性
    3. 播放时钟回调	每帧发出 tick(currentTime) 事件，用于通知外部模块（如 Scheduler、Renderer）进行渲染或调度
    4. 播放结束判断	判断是否到达 duration，根据是否 loop 来决定是停止还是回到起点
    5. 对外统一时间源	提供 getCurrentTime() 等 API，让外部模块查询当前播放时间，作为统一时间基准
    6. 精度与一致性保障	避免播放时间漂移、丢帧等问题，确保时间推进精度可控、行为可预测
 */

type TimeDriverEvent = 'start' | 'pause' | 'stop' | 'tick' | 'ended';
type EventCallback = (time?: number) => void;

export class TimeDriver{

    private listeners = new Map<TimeDriverEvent, Set<EventCallback>>();
    private frameId: number;

    private startTime: number = 0;      // play 开始时刻对应的 performance.now()
    private lastTime: number  = 0;      // 上一次 requestAnimationFrame 的时间
    curTime: number   = 0;      // 当前播放时间（秒）

    duration: number;           // 总时长
    playbackRate: number = 1;   // 播放速率
    playing: boolean = false;   // 播放状态

    constructor(duration: number){
        this.duration = duration;
    }

    setDuration(d: number) { 
        this.duration = d;
        if(this.curTime > d){
            this.curTime = d;
        } 
    }
   
    setPlaybackRate(rate: number) {
        if(rate === 0) throw new Error('[ TimeDriver ] playbackRate cannot be zero');
        
        if(this.playing){
            // 变速时调整基准时间，保持时间连续
            const now = performance.now();
            this.startTime = now - this.curTime / rate;
            this.lastTime = now;
        }
        this.playbackRate = rate;
    }

    on(event: TimeDriverEvent, cb: EventCallback){
        if(!this.listeners.has(event)) this.listeners.set(event, new Set());
        this.listeners.get(event)!.add(cb);
    }

    off(event: TimeDriverEvent, cb: EventCallback){
        this.listeners.get(event)?.delete(cb);
    }

    private emit(event: TimeDriverEvent, time?: number){
        this.listeners.get(event)?.forEach(cb => cb(time));
    }

    play(): void{
        if(this.playing) return;
        this.playing = true;

        const now = performance.now();
        this.startTime = now - this.curTime / this.playbackRate;
        this.lastTime = now;
        this.emit('start', this.curTime);
        this.frameId = requestAnimationFrame(this.tick);
    }

    pause(): void{
        if(!this.playing) return;
        this.playing = false;

        if(this.frameId !== null) cancelAnimationFrame(this.frameId);
        this.frameId = null;
        this.emit('pause', this.curTime);
    }

    stop(): void{
        this.pause();
        this.curTime = 0;
        this.emit('stop', this.curTime);
    }

    seek(time: number){
        this.curTime = Math.min(Math.max(time, 0), this.duration);

        const now = performance.now();
        // 重新计算基准时间，保证时间连续
        this.startTime = now - this.curTime / this.playbackRate;
        this.lastTime = now;
        this.emit('tick', this.curTime);

        // 边界检查，自动触发结束事件并暂停
        if((this.curTime === 0 && this.playbackRate < 0) ||
            (this.curTime === this.duration && this.playbackRate > 0)){
            this.emit('ended', this.curTime);
            this.pause();
        }
    }

    private tick = () => {
        if(!this.playing) return;
        
        const now = performance.now();
        // 根据 playbackRate 正负推进或倒退时间
        const delta = (now - this.lastTime) * this.playbackRate;
        this.curTime += delta;

        this.lastTime = now;

        // 边界处理
        if(this.curTime >= this.duration){
            this.curTime = this.duration;
            this.emit('tick', this.curTime);
            this.emit('ended', this.curTime);
            this.stop();
            return;
        }

        if(this.curTime <= 0){
            this.curTime = 0;
            this.emit('tick', this.curTime);
            this.emit('ended', this.curTime);
            this.stop();
            return;
        }

        this.emit('tick', this.curTime);
        this.frameId = requestAnimationFrame(this.tick);
    }
}