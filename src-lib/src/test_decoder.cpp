#include <iostream>
#include <format>
#include "decoder.h"

using namespace std;

int main() {
    FFMpegVersion v = get_ffmpeg_version();

    cout << std::format("{} {} {}", v.major, v.minor, v.micro) << endl;
    show_frames();
    while (true) {

    }
    return 0;
}