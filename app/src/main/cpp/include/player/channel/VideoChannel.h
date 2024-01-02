#ifndef FFMPEGVIDEO_VIDEOCHANNEL_H
#define FFMPEGVIDEO_VIDEOCHANNEL_H

#include "BaseChannel.h"
#include <android/native_window_jni.h>

extern "C" {
#include "ffmpeg/libswscale/swscale.h"
#include "ffmpeg/libavutil/imgutils.h"
#include "ffmpeg/libavutil/time.h"
#include "ffmpeg/libavutil/parseutils.h"
};

class VideoChannel final : public BaseChannel {

private:
    int video_fps = -1;

    pthread_t video_pid;

    ANativeWindow *aNativeWindow = nullptr;

public:
    VideoChannel(int index, AVCodecContext *context, AVStream *avStream,
                 VideoPlayerCallback *playerCallback, ANativeWindow *window);

    ~VideoChannel();

    virtual void getAVPacket(AVPacket **value) override;

    void renderVideo(uint8_t *srcData, int width, int height, int linesize);

    ANativeWindow *getWindow();

    int getFps();

protected:
    virtual void startPlay() override;

    virtual void pausePlay() override;

    virtual void stopPlay() override;
};


#endif //FFMPEGVIDEO_VIDEOCHANNEL_H
