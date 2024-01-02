#ifndef FFMPEGVIDEO_VIDEOPLAYERCALLBACK_H
#define FFMPEGVIDEO_VIDEOPLAYERCALLBACK_H

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
};

class VideoPlayerCallback {

public:
    virtual void setAudioPlayTime(double time) = 0;

    virtual double getAudioPlayTime() = 0;

    virtual void getVideoAVPacket(AVPacket **value) = 0;

    virtual void getAudioAVPacket(AVPacket **value) = 0;

};

#endif //FFMPEGVIDEO_VIDEOPLAYERCALLBACK_H
