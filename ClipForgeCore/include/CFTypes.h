// #pragma once

// #include <mutex>
// #include <condition_variable>
// #include <memory>
// #include <vector>
// #include <string>
// #include <deque>

// // 单帧数据
// struct CFFrame {
//     int width;
//     int height;
//     int channels;
//     int length;
//     std::shared_ptr<uint8_t[]> data;
//     double timestamp;
// };

// // 帧缓存结构
// struct CFFrameCacheEntry {
//     double timestamp;
//     std::shared_ptr<CFFrame> frame;
// };

// enum class CFStreamType {
//     Video,
//     Audio,
//     Other
// };

// // 媒体流信息
// struct CFStreamInfo {
//     CFStreamType type;
//     int codec_id;
//     std::string codec_name;
//     int width;
//     int height;
//     double fps;
//     int sample_rate;
// };

// // 文件级元数据
// struct CFMetadata {
//     std::string file_path;
//     double duration;
//     int bit_rate;
//     std::vector<CFStreamInfo> infos;
// };