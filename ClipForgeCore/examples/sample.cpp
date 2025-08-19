#include "CFWrapper.h"
#include "CFPlayer.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

using namespace std;

int main()
{

    cf_player_open("E://123.MP4");

    double fps = 30.0;
    int interval = 16;
    double timestamp = 0;


    this_thread::sleep_for(chrono::milliseconds(1000));

    while(true){
        auto frame = cf_player_get_frame_at(timestamp);
        if(frame) {
            cout << "cf_player_get_frame_at : " << frame->timestamp << endl;
            cv::Mat img(frame->height, frame->width, CV_8UC4, frame->data.get());
            cv::imshow("img", img);
            cv::waitKey(1);
            
        }
        
        this_thread::sleep_for(std::chrono::milliseconds(interval));
        timestamp += interval;
           
    }
    return 0;
}