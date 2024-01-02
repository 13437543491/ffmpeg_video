#ifndef FFMPEGVIDEO_AUDIOCHANNEL_H
#define FFMPEGVIDEO_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include "ffmpeg/libswresample/swresample.h"
#include "ffmpeg/libavutil/time.h"
};

class AudioChannel final : public BaseChannel {

private:
    SwrContext *swrContext = nullptr;

    uint8_t *out_buffer = nullptr;

    SLObjectItf audioEngine = nullptr;

    SLEngineItf engineInterface = nullptr;

    SLObjectItf audioPlayer = nullptr;

    SLPlayItf audioPlayerInterface = nullptr;

    SLObjectItf audioOutputMix = nullptr;

    SLAndroidSimpleBufferQueueItf audioBufferQueue = nullptr;

    pthread_t audio_pid;

    int out_sample_rate;

    int out_sample_size;

    int out_channels;

public:
    AudioChannel(int index, AVCodecContext *context, AVStream *avStream,
                 VideoPlayerCallback *playerCallback);

    ~AudioChannel();

    void createOpenSLES();

    SwrContext *getSwrContext();

    uint8_t *getOutBuffer();

    int getOutSampleRate();

    int getOutSampleSize();

    int getOutChannels();

    virtual void getAVPacket(AVPacket **value) override;

protected:
    virtual void startPlay() override;

    virtual void pausePlay() override;

    virtual void stopPlay() override;


};

#endif //FFMPEGVIDEO_AUDIOCHANNEL_H
