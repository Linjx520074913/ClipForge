#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return -1;
    }

    const char* filename = argv[1];

    // 初始化网络组件（有些格式需要）
    avformat_network_init();

    // 打开媒体文件上下文
    AVFormatContext* fmt_ctx = nullptr;
    if (avformat_open_input(&fmt_ctx, filename, nullptr, nullptr) < 0) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return -1;
    }

    // 获取流信息
    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
        std::cerr << "Failed to find stream info" << std::endl;
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    // 打印文件整体信息
    std::cout << "Format: " << fmt_ctx->iformat->name << ", duration: "
                << fmt_ctx->duration / (double)AV_TIME_BASE << " seconds\n";
    std::cout << "Number of streams: " << fmt_ctx->nb_streams << std::endl;

    // 遍历所有流，查找视频流并输出信息
    for (unsigned i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;

        std::cout << "Stream #" << i << " - ";

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "Video stream\n";
            std::cout << "  Codec ID: " << codecpar->codec_id << "\n";
            std::cout << "  Width x Height: " << codecpar->width << "x" << codecpar->height << "\n";

            // 查找解码器名字
            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) {
                std::cout << "  Codec name: " << codec->name << "\n";
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            std::cout << "Audio stream\n";
            std::cout << "  Codec ID: " << codecpar->codec_id << "\n";

            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) {
                std::cout << "  Codec name: " << codec->name << "\n";
            }
        } else {
            std::cout << "Other stream type\n";
        }
    }

    avformat_close_input(&fmt_ctx);
    avformat_network_deinit();

    return 0;
}
    