#include "CFDecoder.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

using namespace std;

int main()
{
    CFDecoder decoder;
    decoder.open_video("E://test.MP4");

    double fps = 30.0;
    double interval = 1.0 / fps * 1000;
    double timestamp = 0.0;

    while(true){
        auto frame = decoder.get_frame(timestamp);
        if(frame) {
            cout << "Frame : " << frame->timestamp << endl;
            cv::Mat img(frame->height, frame->width, CV_8UC4, frame->data.get());
            cv::imshow("img", img);
            cv::waitKey(1);
        }

        timestamp += interval;
        std::this_thread::sleep_for(std::chrono::milliseconds((int)interval));
    }
    return 0;
}