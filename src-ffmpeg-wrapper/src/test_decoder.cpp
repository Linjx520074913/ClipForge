#include "video_decoder.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    VideoDecoder decoder;
    decoder.Initialize("E://test.MP4");

    cv::namedWindow("Video Playback", cv::WINDOW_AUTOSIZE);

    int64_t current_time_ms = 0;
    int frame_interval_ms = 20; // 25fps
    while(true) {
        auto frame = decoder.GetFrame(current_time_ms, 100);
        if(!frame) {
            std::cout << "Frame not ready at " << current_time_ms << " ms\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        cv::imshow("Video Playback", *frame);
        cv::waitKey(10);

        current_time_ms += frame_interval_ms;
        // std::cout << "======== " << current_time_ms << std::endl;
    }

    decoder.Uninitialize();

    return 0;
}