#include "timeline_clock.h"

using namespace std;
using namespace std::chrono;

void TimelineClock::play()
{
    if(playing_) return;

    playing_ = true;

    auto now = high_resolution_clock::now();
    auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

    start_time_ = now_ms - current_time_ms_ / speed_;
    last_time_  = start_time_;

    tick_thread_ = std::thread(&TimelineClock::tick_loop, this);
}

void TimelineClock::pause()
{
    if(!playing_) return;

    playing_ = false;
}

void TimelineClock::stop()
{
    pause();
    current_time_ms_ = 0;
}

void TimelineClock::tick_loop()
{
    while(playing_) {
        auto now = high_resolution_clock::now();
        auto now_ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

        long delta = (now_ms - last_time_) * speed_;
        current_time_ms_ += delta;

        last_time_ = now_ms;

        // 边界处理
        if(current_time_ms_ >= duration_) {
            current_time_ms_ = duration_;
            stop();
            return;
        }

        if(current_time_ms_ <= 0) {
            current_time_ms_ = 0;
            return;
        }

        if (current_time_ms_ > 10 * 1000) {
            set_speed(2);
        }
        
        if(callback_) {
            callback_(current_time_ms_);
        }
    }
}