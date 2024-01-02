#ifndef FFMPEGVIDEO_BASECHANNEL_H
#define FFMPEGVIDEO_BASECHANNEL_H

#include "player/VideoPlayerCallback.h"
#include "utils/LogUtil.h"
#include <pthread.h>
#include "utils/AVFrameQueue.h"
#include "player/VideoPlayerState.h"
#include "utils/release_util.h"

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavformat/avformat.h"
};

class BaseChannel : public VideoPlayerState {

private:
    int stream_index;

    AVCodecContext *avCodecContext;

    AVStream *avStream = nullptr;

    pthread_mutex_t state_mutex;

    pthread_cond_t state_cond;

    AVFrameQueue avFrameQueue;

    pthread_t pid_avPacket;

    int play_state = STATE_STOP;

private:
    void release();

protected:
    virtual void startPlay() = 0;

    virtual void pausePlay() = 0;

    virtual void stopPlay() = 0;

public:
    VideoPlayerCallback *videoPlayerCallback = nullptr;

    BaseChannel(int index, AVCodecContext *context, AVStream *avStream,
                VideoPlayerCallback *playerCallback);

    virtual ~BaseChannel();

    void start();

    void pause();

    void stop();

    void pushAVFrame(AVFrame *avFrame);

    void getAVFrame(AVFrame **value);

    virtual bool isPlaying() override;

    virtual bool isPause() override;

    virtual bool isStop() override;

    virtual bool isDestroy() override;

    int getStreamIndex();

    AVCodecContext *getAVCodecContext();

    AVStream *getAVStream();

    AVRational getTimeBase();

    void wait();

    virtual void getAVPacket(AVPacket **value) = 0;
};


#endif //FFMPEGVIDEO_BASECHANNEL_H
