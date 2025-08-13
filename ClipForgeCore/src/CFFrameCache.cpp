#include "CFFrameCache.h"

#include <algorithm>
#include <cmath>
#include <iostream>

void CFFrameCache::push(double timestamp, std::shared_ptr<CFFrame> frame)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 删除最旧数据帧
    if(cache_.size() >= capacity_) {
        std::cout << "delete oldest frame" << std::endl;
        cache_.pop_front();
    }

    cache_.emplace_back(CFFrameCacheEntry{ timestamp, frame });
}

std::shared_ptr<CFFrame> CFFrameCache::get_frame(double timestamp)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if(cache_.empty()) return nullptr;

    auto it = std::min_element(cache_.begin(), cache_.end(),
        [timestamp](const CFFrameCacheEntry&a, const CFFrameCacheEntry& b){
            return std::abs(a.timestamp - timestamp) < std::abs(b.timestamp - timestamp);
        }
    );

    int max_diff = 0.03 * 1000; // 根据帧率设置，0.03 对应 33 FPS
    if(std::abs(it->timestamp - timestamp) > max_diff) {
        return nullptr;
    }

    return it->frame;
}