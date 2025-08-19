#include "video_decoder.h"
#include "clipforge_decoder.h"
#include "ffmpeg_wrapper.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    // get_av_meta_data("E://test.MP4");
    CFDecoder decoder;
    decoder.open_video("E://test.MP4");
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
        CFFrame* frame = decoder.get_frame();
        if(frame){
            cv::Mat mat(frame->height, frame->width, CV_8UC4, frame->data);

            cv::imshow("Video", mat);
            cv::waitKey(1);
            decoder.free_frame(frame);
        }
    }
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
