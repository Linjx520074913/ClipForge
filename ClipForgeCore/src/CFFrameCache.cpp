// #include "CFFrameCache.h"

// #include <algorithm>
// #include <cmath>
// #include <iostream>

// void CFFrameCache::push(double timestamp_ms, std::shared_ptr<CFFrame> frame)
// {
//     std::unique_lock<std::mutex> lock(mutex_);
    
//     // 超过这个时间的旧帧丢弃
//     double drop_threshold_ms = 1000.0; 
//     while (!cache_.empty() && cache_.front().timestamp < timestamp_ms - drop_threshold_ms) {
//         cache_.pop_front();
//     }

//     cache_.push_back(CFFrameCacheEntry{ timestamp_ms, frame });
//     cv_.notify_all();
// }

// std::shared_ptr<CFFrame> CFFrameCache::get_nearest_frame(double timestamp_ms)
// {
//     std::unique_lock<std::mutex> lock(mutex_);

//     if(cache_.empty()) return nullptr;

//     auto cmp = [timestamp_ms](const CFFrameCacheEntry& a, const CFFrameCacheEntry& b) {
//         return a.timestamp < b.timestamp;
//     };

//     // 找到第一个时间戳不小于 timestamp_ms 的元素
//     CFFrameCacheEntry key{ timestamp_ms, nullptr };
//     auto it = std::lower_bound(cache_.begin(), cache_.end(), key, cmp);

//     std::shared_ptr<CFFrame> result;

//     if(it == cache_.begin()) {
//         result = it->frame;
//     }
//     else if(it == cache_.end()) {
//         result = (cache_.end() - 1)->frame;
//     }else {
//         // 比较 it 和 it 的前一帧谁更接近 timestamp_ms
//         auto prev = it - 1;
//         result = (std::abs(prev->timestamp - timestamp_ms) < std::abs(it->timestamp - timestamp_ms))
//         ? prev->frame : it->frame;
//     }

//     // 最大允许偏差（毫秒），比如 33ms = 30 fps
//     double max_diff_ms = 33.0;
//     if(std::abs(result->timestamp - timestamp_ms) > max_diff_ms) {
//         return nullptr;
//     }
   
//     return result; 
// }

// std::shared_ptr<CFFrame> CFFrameCache::get_frame(double timestamp)
// {
//     clean_old_frames(timestamp);

//     need_decode_ = need_decode(timestamp);
//     if(need_decode_){

//     }else {

//     }
//     // 如果要获取的时间戳超出缓存范围，则清空缓存
//     // 如果缓存中有帧，则返回最近的帧

//     // 如果缓存中没有帧，则开始下一轮解码
// }

// bool CFFrameCache::need_decode(double timestamp)
// {
//     if(cache_.empty()
//     || (timestamp > cache_.back().timestamp - window_size / 2)
//     || (timestamp < cache_.front().timestamp + window_size / 2)){
//         return true;
//     }
//     return false;
// }

// void CFFrameCache::clean_old_frames(double timestamp)
// {

// }

// double CFFrameCache::get_duration()
// {
//     if(cache_.empty()) return 0;
//     return cache_.back().timestamp - cache_.front().timestamp;
// }

// void CFFrameCache::wait_for_low()
// {
//     std::unique_lock<std::mutex> lock(mutex_);
//     cv_.wait(lock, [this] {
//         return get_duration() <= 500;
//     });
// }

// void CFFrameCache::wait_for_high()
// {
//     std::unique_lock<std::mutex> lock(mutex_);
//     cv_.wait(lock, [this] {
//         return get_duration() >= 3000;
//     });
// }