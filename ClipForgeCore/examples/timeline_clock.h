#pragma once

#include <chrono>
#include <atomic>
#include <thread>
#include <iostream>
#include <functional>

/**
 * 全局播放时钟，定时触发 tick
 * 时间驱动的主要职责
 *  1. 播放时间推进	基于 performance.now() 与 requestAnimationFrame，计算每帧的 deltaTime 并更新 currentTime
    2. 播放控制状态管理	控制播放（start）、暂停（pause）、停止（stop）、跳转（seek）状态切换与逻辑正确性
    3. 播放时钟回调	每帧发出 tick(currentTime) 事件，用于通知外部模块（如 Scheduler、Renderer）进行渲染或调度
    4. 播放结束判断	判断是否到达 duration，根据是否 loop 来决定是停止还是回到起点
    5. 对外统一时间源	提供 getCurrentTime() 等 API，让外部模块查询当前播放时间，作为统一时间基准
    6. 精度与一致性保障	避免播放时间漂移、丢帧等问题，确保时间推进精度可控、行为可预测
 */

enum TimeStatus {
    START,
    PAUSE,
    STOP,
    TICK,
    ENDED
};

using TickCallback = std::function<void(double current_time)>;

class TimelineClock
{
public:
    TimelineClock(int d = 100 * 1000) : duration_(d) {
        current_time_ms_ = 0;
        delta_ms_ = 30;
        speed_ = 1;
        playing_ = false;
    }
    ~TimelineClock() { }   

    void set_duration(int duration) { duration_ = duration; }
    void set_speed(double speed) { speed_ = speed; }
    void set_tick(TickCallback callback) { callback_ = callback; }

    void play();
    void pause();
    void stop();

private:
    void tick_loop();

private:

    double start_time_;      //
    double last_time_;       //

    double speed_;           // 播放速率
    double duration_;        // 总时长
    double current_time_ms_; // 当前播放时间（毫秒）
    bool playing_;           // 播放状态
    double delta_ms_;         // 时间间隔

    std::thread tick_thread_;
    std::atomic<bool> tick_thread_running_;

    TickCallback callback_;
};