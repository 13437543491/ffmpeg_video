#ifndef FFMPEGVIDEO_VIDEOPLAYERIMPL_H
#define FFMPEGVIDEO_VIDEOPLAYERIMPL_H

#include <android/native_window_jni.h>
#include <cstring>
#include <pthread.h>
#include "utils/AVPacketQueue.h"
#include "utils/AVFrameQueue.h"
#include "utils/LogUtil.h"
#include "player/channel/VideoChannel.h"
#include "player/channel/AudioChannel.h"
#include "player/VideoPlayerCallback.h"
#include "VideoPlayerState.h"

extern "C" {
#include "ffmpeg/libavformat/avformat.h"
};

class VideoPlayerImpl final : public VideoPlayerCallback, VideoPlayerState {

private:
    AVPacketQueue videoAVPacketQueue;

    AVPacketQueue audioAVPacketQueue;

    char *sourcePath = nullptr;

    ANativeWindow *aNativeWindow;

    AVFormatContext *avFormatContext = nullptr;

    VideoChannel *videoChannel = nullptr;

    AudioChannel *audioChannel = nullptr;

    double audioPlayTime = -1;

    int play_state = STATE_STOP;

    pthread_mutex_t read_mutex;

    pthread_cond_t read_cond;

    pthread_t read_pid;

    void release();

public:
    VideoPlayerImpl();

    ~VideoPlayerImpl();

    void playVideo(const char *path, ANativeWindow *window);

    void pause();

    void stop();

    ANativeWindow *getWindow();

    char *getSourcePath();

    void pushVideoAVPacket(AVPacket *value);

    void pushAudioAVPacket(AVPacket *value);

    void initPlayChannel(AVFormatContext *context, VideoChannel *vChannel,
                         AudioChannel *aChannel);

    virtual void setAudioPlayTime(double time) override;

    virtual double getAudioPlayTime() override;

    virtual void getVideoAVPacket(AVPacket **value) override;

    virtual void getAudioAVPacket(AVPacket **value) override;

    virtual bool isPlaying() override;

    virtual bool isPause() override;

    virtual bool isStop() override;

    virtual bool isDestroy() override;

    void waitPlay();

    void resumePlay();
};


#endif //FFMPEGVIDEO_VIDEOPLAYERIMPL_H
