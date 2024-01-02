#include "player/channel/BaseChannel.h"


BaseChannel::BaseChannel(int index, AVCodecContext *context, AVStream *avStream,
                         VideoPlayerCallback *playerCallback)
        : stream_index(index), avCodecContext(context), avStream(avStream),
          videoPlayerCallback(playerCallback) {

}

void BaseChannel::pushAVFrame(AVFrame *avFrame) {
    this->avFrameQueue.push(avFrame);
}

void BaseChannel::getAVFrame(AVFrame **value) {
    this->avFrameQueue.get(value);
}

bool BaseChannel::isPlaying() {
    return this->play_state == STATE_START;
}

bool BaseChannel::isPause() {
    return this->play_state == STATE_PAUSE;
}

bool BaseChannel::isStop() {
    return this->play_state == STATE_STOP;
}

int BaseChannel::getStreamIndex() {
    return this->stream_index;
}

AVCodecContext *BaseChannel::getAVCodecContext() {
    return this->avCodecContext;
}

AVStream *BaseChannel::getAVStream() {
    return this->avStream;
}

void *videoAVPacketHandle(void *args) {
    BaseChannel *thiz = static_cast<BaseChannel *>(args);
    AVCodecContext *avCodecContext = thiz->getAVCodecContext();

    AVPacket *avPacket = nullptr;
//     = nullptr;
    while (!thiz->isStop()) {
        thiz->getAVPacket(&avPacket);
        if (thiz->isPause()) {
            thiz->wait();
        }

        if (thiz->isStop()) {
            break;
        }

        if (avPacket == nullptr) {
            continue;
        }

        avcodec_send_packet(avCodecContext, avPacket);
        releaseAVPacket(&avPacket);

        AVFrame *avFrame = av_frame_alloc();
        int ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {
            thiz->pushAVFrame(avFrame);
        } else {
            releaseAVFrame(&avFrame);
            if (ret == AVERROR(EAGAIN)) {
                continue;
            } else {
                break;
            }
        }
    }

    releaseAVPacket(&avPacket);
    return nullptr;
}

void BaseChannel::start() {
    if (this->play_state != STATE_START) {
        bool isPause = this->isPause();
        this->play_state = STATE_START;
        if (isPause) {
            pthread_cond_broadcast(&this->state_cond);
        } else {
            this->avFrameQueue.init();
            pthread_mutex_init(&this->state_mutex, nullptr);
            pthread_cond_init(&this->state_cond, nullptr);
            pthread_create(&pid_avPacket, nullptr, videoAVPacketHandle, this);
            this->startPlay();
        }
    }
}

void BaseChannel::pause() {
    if (this->play_state == STATE_START) {
        this->play_state = STATE_PAUSE;
        pausePlay();
    }
}

void BaseChannel::stop() {
    if (this->play_state == STATE_STOP) {
        return;
    }
    this->play_state = STATE_STOP;
    this->release();
}

AVRational BaseChannel::getTimeBase() {
    return this->avStream->time_base;
}

void BaseChannel::wait() {
    pthread_mutex_lock(&this->state_mutex);
    pthread_cond_wait(&this->state_cond, &this->state_mutex);
    pthread_mutex_unlock(&this->state_mutex);
}

BaseChannel::~BaseChannel() {
    this->release();
}

void BaseChannel::release() {
    if (this->isDestroy()) {
        return;
    }
    this->avFrameQueue.destroy();
    pthread_cond_broadcast(&this->state_cond);
    stopPlay();
    pthread_join(pid_avPacket, nullptr);
    pthread_mutex_destroy(&this->state_mutex);
    pthread_cond_destroy(&this->state_cond);
    if (this->avCodecContext) {
        avcodec_free_context(&this->avCodecContext);
    }
    this->avStream = nullptr;
    this->play_state = STATE_DESTROY;
}

bool BaseChannel::isDestroy() {
    return this->play_state == STATE_DESTROY;
}






