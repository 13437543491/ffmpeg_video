#include "player/channel/AudioChannel.h"

AudioChannel::AudioChannel(int index, AVCodecContext *context, AVStream *avStream,
                           VideoPlayerCallback *playerCallback)
        : BaseChannel(index, context, avStream, playerCallback) {
    this->out_sample_rate = 44100;
    this->out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    this->out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    int bufferSize = this->out_sample_rate * this->out_sample_size * this->out_channels;
    this->out_buffer = static_cast<uint8_t *>(malloc(bufferSize));

    this->swrContext = swr_alloc_set_opts(nullptr,
                                          AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                          this->out_sample_rate,
                                          context->channel_layout, context->sample_fmt,
                                          context->sample_rate, 0, nullptr
    );
    swr_init(this->swrContext);
}

void *createOpenSLES_(void *args);

void audioPlayCallback(SLAndroidSimpleBufferQueueItf caller, void *args);

void AudioChannel::startPlay() {
    pthread_create(&audio_pid, nullptr, createOpenSLES_, this);
}

void *createOpenSLES_(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->createOpenSLES();
    return nullptr;
}

void AudioChannel::createOpenSLES() {
    SLresult ret = slCreateEngine(&audioEngine, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("创建播放器引擎失败:%d", ret);
        return;
    }

    ret = (*audioEngine)->Realize(audioEngine, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("引擎初始化失败:%d", ret);
        return;
    }

    ret = (*audioEngine)->GetInterface(audioEngine, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("获取引擎接口失败:%d", ret);
        return;
    }


    ret = (*engineInterface)->CreateOutputMix(engineInterface, &audioOutputMix, 0, nullptr,
                                              nullptr);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("创建混音器失败:%d", ret);
        return;
    }

    ret = (*audioOutputMix)->Realize(audioOutputMix, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("混音器初始化失败:%d", ret);
        return;
    }


    SLDataLocator_AndroidSimpleBufferQueue pLocator = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            10
    };
    SLDataFormat_PCM pFormat = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource pAudioSrc = {&pLocator, &pFormat};

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, audioOutputMix};
    SLDataSink pAudioSnk = {&outputMix, nullptr};

    const SLInterfaceID pInterfaceIds[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean pInterfaceRequired[1] = {SL_BOOLEAN_TRUE};

    ret = (*engineInterface)->CreateAudioPlayer(
            engineInterface,
            &audioPlayer,
            &pAudioSrc,
            &pAudioSnk,
            1,
            pInterfaceIds,
            pInterfaceRequired
    );
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("创建播放器失败:%d", ret);
        return;
    }

    ret = (*audioPlayer)->Realize(audioPlayer, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("播放器初始化失败:%d", ret);
        return;
    }

    ret = (*audioPlayer)->GetInterface(audioPlayer, SL_IID_PLAY, &audioPlayerInterface);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("获取播放接口失败:%d", ret);
        return;
    }

    ret = (*audioPlayer)->GetInterface(audioPlayer, SL_IID_BUFFERQUEUE, &audioBufferQueue);
    if (SL_RESULT_SUCCESS != ret) {
        LOGE("获取播放队列失败:%d", ret);
        return;
    }

    (*audioBufferQueue)->RegisterCallback(audioBufferQueue, audioPlayCallback, this);
    (*audioPlayerInterface)->SetPlayState(audioPlayerInterface, SL_PLAYSTATE_PLAYING);
    audioPlayCallback(audioBufferQueue, this);

}

void audioPlayCallback(SLAndroidSimpleBufferQueueItf queue, void *args) {

    AudioChannel *thiz = static_cast<AudioChannel *>(args);
    AVFrame *avFrame = nullptr;
    thiz->getAVFrame(&avFrame);

    if (thiz->isPause()) {
        thiz->wait();
    }
    if (thiz->isStop()) {
        releaseAVFrame(&avFrame);
        return;
    }
    if (avFrame == nullptr) {
        return;
    }

    SwrContext *swrContext = thiz->getSwrContext();

    int dst_nb_samples = av_rescale_rnd(
            swr_get_delay(swrContext, avFrame->sample_rate) + avFrame->nb_samples,
            thiz->getOutSampleRate(), // 输出采样率
            avFrame->sample_rate, // 输入采样率
            AV_ROUND_UP
    );

    uint8_t *out_buffer = thiz->getOutBuffer();
    int samples_per_channel = swr_convert(swrContext,
                                          &out_buffer, dst_nb_samples,
                                          (const uint8_t **) avFrame->data, avFrame->nb_samples
    );

    AVRational timeBase = thiz->getTimeBase();
    double audio_play_time = avFrame->best_effort_timestamp * av_q2d(timeBase);
    thiz->videoPlayerCallback->setAudioPlayTime(audio_play_time);

    releaseAVFrame(&avFrame);

    int out_sample_size = thiz->getOutSampleSize();
    int out_channels = thiz->getOutChannels();
    int pcm_data_size = samples_per_channel * out_sample_size * out_channels;
    (*queue)->Enqueue(queue, out_buffer, pcm_data_size);
}

void AudioChannel::pausePlay() {

}

void AudioChannel::stopPlay() {
    pthread_join(audio_pid, nullptr);

    if (audioPlayerInterface) {
        (*audioPlayerInterface)->SetPlayState(audioPlayerInterface, SL_PLAYSTATE_STOPPED);
        audioPlayerInterface = nullptr;
    }

    // 7.2 销毁播放器
    if (audioPlayer) {
        (*audioPlayer)->Destroy(audioPlayer);
        audioPlayer = nullptr;
        audioBufferQueue = nullptr;
    }

    // 7.3 销毁混音器
    if (audioOutputMix) {
        (*audioOutputMix)->Destroy(audioOutputMix);
        audioOutputMix = nullptr;
    }

    // 7.4 销毁引擎
    if (audioEngine) {
        (*audioEngine)->Destroy(audioEngine);
        audioEngine = nullptr;
        engineInterface = nullptr;
    }

    if (this->swrContext) {
        swr_free(&this->swrContext);
    }

    if (this->out_buffer) {
        delete this->out_buffer;
        this->out_buffer = nullptr;
    }
}

SwrContext *AudioChannel::getSwrContext() {
    return this->swrContext;
}

uint8_t *AudioChannel::getOutBuffer() {
    return this->out_buffer;
}

int AudioChannel::getOutSampleRate() {
    return this->out_sample_rate;
}

int AudioChannel::getOutSampleSize() {
    return this->out_sample_size;
}

int AudioChannel::getOutChannels() {
    return this->out_channels;
}

void AudioChannel::getAVPacket(AVPacket **value) {
    this->videoPlayerCallback->getAudioAVPacket(value);
}

AudioChannel::~AudioChannel() {
}
