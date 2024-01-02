#ifndef FFMPEGVIDEO_RELEASE_UTIL_H
#define FFMPEGVIDEO_RELEASE_UTIL_H

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
};

static void releaseAVPacket(AVPacket **packet) {
    if (*packet) {
        av_packet_unref(*packet);
        av_packet_free(packet);
        *packet = nullptr;
    }
}

static void releaseAVFrame(AVFrame **avFrame) {
    if (*avFrame) {
        av_frame_unref(*avFrame);
        av_frame_free(avFrame);
        *avFrame = nullptr;
    }
}

#endif //FFMPEGVIDEO_RELEASE_UTIL_H
