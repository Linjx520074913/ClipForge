#pragma once

#include <deque>
#include <mutex>

#include "CFTypes.h"

class CFFrameCache
{
public:
    CFFrameCache(size_t capacity = 100) : capacity_(capacity) {}

    void push(double timestamp, std::shared_ptr<CFFrame> frame);

    std::shared_ptr<CFFrame> get_frame(double timestamp);

private:
    std::mutex mutex_;

    size_t capacity_;
    std::deque<CFFrameCacheEntry> cache_;
};