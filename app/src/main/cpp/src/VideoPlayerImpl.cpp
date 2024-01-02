#include "player/VideoPlayerImpl.h"


VideoPlayerImpl::VideoPlayerImpl() {

}

VideoPlayerImpl::~VideoPlayerImpl() {
    this->release();
}

void *startPlay(void *args);

void VideoPlayerImpl::playVideo(const char *path, ANativeWindow *window) {
    if (this->isPause()) {
        this->play_state = STATE_START;
        this->resumePlay();
        if (this->videoChannel) {
            this->videoChannel->start();
        }
        if (this->audioChannel) {
            this->audioChannel->start();
        }
        return;
    }

    this->sourcePath = new char[strlen(path) + 1];
    strcpy(this->sourcePath, path);

    this->aNativeWindow = window;

    this->videoAVPacketQueue.init();
    this->audioAVPacketQueue.init();

    pthread_mutex_init(&this->read_mutex, nullptr);
    pthread_cond_init(&this->read_cond, nullptr);

    pthread_create(&read_pid, nullptr, startPlay, this);
}

void *startPlay(void *args) {
    VideoPlayerImpl *thiz = static_cast<VideoPlayerImpl *>(args);
    char *sourcePath = thiz->getSourcePath();

    AVFormatContext *avFormatContext = avformat_alloc_context();
    int ret = avformat_open_input(&avFormatContext, sourcePath, nullptr, nullptr);
    if (ret != 0) {
        LOGE("打开文件失败:%s", av_err2str(ret));
        return nullptr;
    }

    ret = avformat_find_stream_info(avFormatContext, nullptr);
    if (ret < 0) {
        LOGE("查找文件流:%s", av_err2str(ret));
        return nullptr;
    }

    VideoChannel *videoChannel = nullptr;
    AudioChannel *audioChannel = nullptr;
    for (int stream_index = 0; stream_index < avFormatContext->nb_streams; ++stream_index) {
        AVStream *avStream = avFormatContext->streams[stream_index];

        AVCodecParameters *avCodecParameters = avStream->codecpar;
        AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
        if (!avCodec) {
            LOGE("找不到解码器");
            return nullptr;
        }

        AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
        if (!avCodecContext) {
            LOGE("创建AVCodecContext失败");
            return nullptr;
        }

        ret = avcodec_parameters_to_context(avCodecContext, avCodecParameters);
        if (ret < 0) {
            LOGE("初始化AVCodecContext失败:%s", av_err2str(ret));
            return nullptr;
        }

        ret = avcodec_open2(avCodecContext, avCodec, nullptr);
        if (ret != 0) {
            LOGE("解码器无法打开:%s", av_err2str(ret));
            return nullptr;
        }

        if (avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (avStream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }
            videoChannel = new VideoChannel(stream_index, avCodecContext, avStream,
                                            thiz, thiz->getWindow());
            videoChannel->start();
        } else if (avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioChannel = new AudioChannel(stream_index, avCodecContext, avStream,
                                            thiz);
            audioChannel->start();
        }
    }
    thiz->initPlayChannel(avFormatContext, videoChannel, audioChannel);

    AVPacket *avPacket = nullptr;
    while (!thiz->isStop()) {
        if (thiz->isPause()) {
            av_read_pause(avFormatContext);
            thiz->waitPlay();
            av_read_play(avFormatContext);
        }

        if (thiz->isStop()) {
            break;
        }

        avPacket = av_packet_alloc();
        ret = av_read_frame(avFormatContext, avPacket);
        if (ret == AVERROR_EOF) {
            break;
        }

        if (ret == 0) {
            if (videoChannel && videoChannel->getStreamIndex() == avPacket->stream_index) {
                thiz->pushVideoAVPacket(avPacket);
            } else if (audioChannel && audioChannel->getStreamIndex() == avPacket->stream_index) {
                thiz->pushAudioAVPacket(avPacket);
            }
        }
    }
    return nullptr;
}

void VideoPlayerImpl::initPlayChannel(AVFormatContext *context, VideoChannel *vChannel,
                                      AudioChannel *aChannel) {
    this->avFormatContext = context;
    this->videoChannel = vChannel;
    this->audioChannel = aChannel;
    this->play_state = STATE_START;
}

ANativeWindow *VideoPlayerImpl::getWindow() {
    return this->aNativeWindow;
}

char *VideoPlayerImpl::getSourcePath() {
    return this->sourcePath;
}

void VideoPlayerImpl::pause() {
    this->play_state = STATE_PAUSE;
    if (this->videoChannel) {
        this->videoChannel->pause();
    }
    if (this->audioChannel) {
        this->audioChannel->pause();
    }
}

void VideoPlayerImpl::stop() {
    this->release();
}

void VideoPlayerImpl::release() {
    if (this->isDestroy()) {
        return;
    }

    this->resumePlay();
    pthread_join(this->read_pid, nullptr);
    pthread_mutex_destroy(&this->read_mutex);
    pthread_cond_destroy(&this->read_cond);

    this->videoAVPacketQueue.destroy();
    this->audioAVPacketQueue.destroy();

    if (avFormatContext) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
    }

    if (this->videoChannel) {
        this->videoChannel->stop();
        delete this->videoChannel;
        this->videoChannel = nullptr;
    }

    if (this->audioChannel) {
        this->audioChannel->stop();
        delete this->audioChannel;
        this->audioChannel = nullptr;
    }

    if (this->sourcePath) {
        delete this->sourcePath;
        this->sourcePath = nullptr;
    }

    if (this->aNativeWindow) {
//        ANativeWindow_release(this->aNativeWindow);
        this->aNativeWindow = nullptr;
    }

    this->play_state = STATE_DESTROY;
}

void VideoPlayerImpl::setAudioPlayTime(double time) {
    this->audioPlayTime = time;
}

double VideoPlayerImpl::getAudioPlayTime() {
    return this->audioPlayTime;
}

void VideoPlayerImpl::getVideoAVPacket(AVPacket **value) {
    this->videoAVPacketQueue.get(value);
}

void VideoPlayerImpl::getAudioAVPacket(AVPacket **value) {
    this->audioAVPacketQueue.get(value);
}

void VideoPlayerImpl::pushVideoAVPacket(AVPacket *value) {
    this->videoAVPacketQueue.push(value);
}

void VideoPlayerImpl::pushAudioAVPacket(AVPacket *value) {
    this->audioAVPacketQueue.push(value);
}

bool VideoPlayerImpl::isPlaying() {
    return this->play_state == STATE_START;
}

bool VideoPlayerImpl::isPause() {
    return this->play_state == STATE_PAUSE;
}

bool VideoPlayerImpl::isStop() {
    return this->play_state == STATE_STOP;
}

bool VideoPlayerImpl::isDestroy() {
    return this->play_state == STATE_DESTROY;
}

void VideoPlayerImpl::waitPlay() {
    if (isPause()) {
        pthread_mutex_lock(&this->read_mutex);
        pthread_cond_wait(&this->read_cond, &this->read_mutex);
        pthread_mutex_unlock(&this->read_mutex);
    }
}

void VideoPlayerImpl::resumePlay() {
    pthread_mutex_lock(&this->read_mutex);
    pthread_cond_signal(&this->read_cond);
    pthread_mutex_unlock(&this->read_mutex);
}


