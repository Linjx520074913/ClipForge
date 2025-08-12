#include "video_decoder.h"
#include "ffmpeg_wrapper.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    get_av_meta_data("E://test.MP4");
    // VideoDecoder decoder;
    // if (!decoder.Initialize("E://test.MP4")) {
    //     std::cerr << "Failed to initialize decoder" << std::endl;
    //     return -1;
    // }

    // cv::namedWindow("Playback", cv::WINDOW_AUTOSIZE);

    // int64_t current_time_ms = 0;
    // const int frame_interval_ms = 33;  // ~30 FPS

    // while (true) {
    //     auto frame = decoder.GetFrame(current_time_ms, 100);
    //     if (!frame) {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //         continue;
    //     }

    //     cv::imshow("Playback", *frame);
    //     int key = cv::waitKey(1);
    //     if (key == 27) break; // ESC退出

    //     current_time_ms += frame_interval_ms;
    // }

    // decoder.Uninitialize();
    // return 0;
}
