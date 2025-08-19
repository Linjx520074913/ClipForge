// #pragma once

// #include <deque>

// #include "CFTypes.h"

// class CFFrameCache
// {
// public:

//     /**
//      * @brief 推入帧
//      * 
//      * @param timestamp 
//      * @param frame 
//      */
//     void push(double timestamp, std::shared_ptr<CFFrame> frame);

//     /**
//      * @brief 查找最近帧
//      * 
//      * @param timestamp 
//      * @return std::shared_ptr<CFFrame> 
//      */
//     std::shared_ptr<CFFrame> get_nearest_frame(double timestamp_ms);

//     // 阻塞直到缓存低于 high 或 高于 low
//     void wait_for_low();
//     void wait_for_high();

//     double get_duration();

//     /**
//      * @brief 获取数据帧
//      * 
//      * @param timestamp 
//      * @return std::shared_ptr<CFFrame> 
//      */
//     std::shared_ptr<CFFrame> get_frame(double timestamp);

// private:
//     /**
//      * @brief 获取时间戳最近的帧
//      * 
//      * @param timestamp 
//      * @return std::shared_ptr<CFFrame> 
//      */
//     std::shared_ptr<CFFrame> get_nearest_frame(double timestamp);
    
//     /**
//      * @brief 是否需要解码
//      * 
//      * @param timestamp 
//      * @return true 
//      * @return false 
//      */
//     bool need_decode(double timestamp);

//     /**
//      * @brief 清理过时帧
//      * 
//      * @param timestamp 
//      */
//     void clean_old_frames(double timestamp);

//     double buffer_time_range() {
//         if(cache_.empty()) return 0.0;

//         return cache_.back().timestamp - cache_.front().timestamp;
//     }

// private:
//     std::mutex mutex_;
//     std::condition_variable cv_;

//     size_t max_timestamp_diff_ = 4 * 1000; // 允许最大的时间差，单位毫秒
//     size_t window_size = max_timestamp_diff_ / 2;
//     bool need_decode_ = false; // 是否需要解码

//     std::deque<CFFrameCacheEntry> cache_;
// };