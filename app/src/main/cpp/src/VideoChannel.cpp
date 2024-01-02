#include "player/channel/VideoChannel.h"

VideoChannel::VideoChannel(int index, AVCodecContext *context, AVStream *avStream,
                           VideoPlayerCallback *playerCallback, ANativeWindow *window)
        : BaseChannel(index, context, avStream, playerCallback), aNativeWindow(window) {
    AVRational avRational = this->getAVStream()->avg_frame_rate;
    video_fps = (int) av_q2d(avRational);
}

int VideoChannel::getFps() {
    return this->video_fps;
}

void *videoAVFrameDecode(void *args);

void VideoChannel::startPlay() {
    pthread_create(&video_pid, nullptr, videoAVFrameDecode, this);
}

void *videoAVFrameDecode(void *args) {
    VideoChannel *thiz = static_cast<VideoChannel *>(args);
    AVCodecContext *avCodecContext = thiz->getAVCodecContext();

    ANativeWindow *window = thiz->getWindow();
    int window_width = ANativeWindow_getWidth(window);
    int window_height = ANativeWindow_getHeight(window);

    int srcW = avCodecContext->width;
    int srcH = avCodecContext->height;

    int dstW = 0;
    int dstH = 0;
    bool isPortrait = window_height > window_width;
    if (srcW > srcH) {
        if (isPortrait) {
            dstW = window_width;
            dstH = (dstW * srcH) / srcW;
        } else {
            dstH = window_height;
            dstW = (srcW * window_height) / srcH;
            if (dstW > window_width) {
                dstW = window_width;
                dstH = (dstW * srcH) / srcW;
            }
        }
    } else {
        if (isPortrait) {
            if (srcH >= window_height) {
                dstH = window_height;
                dstW = (srcW * window_height) / srcH;
            } else {
                dstW = window_width;
                dstH = (dstW * srcH) / srcW;

                if (dstH > window_height) {
                    dstH = window_height;
                    dstW = (srcW * window_height) / srcH;
                }
            }
        } else {
            dstH = window_height;
            dstW = (srcW * window_height) / srcH;
        }
    }

    SwsContext *swsContext = sws_getContext(
            avCodecContext->width, avCodecContext->height, avCodecContext->pix_fmt,
            dstW, dstH, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr
    );

    uint8_t *dst_data[4];
    int dst_stride[4];
    av_image_alloc(dst_data, dst_stride, dstW, dstH, AV_PIX_FMT_RGBA, 1);

    int fps = thiz->getFps();
    AVRational timeBase = thiz->getTimeBase();

    AVFrame *avFrame = nullptr;
    while (!thiz->isStop()) {
        thiz->getAVFrame(&avFrame);
        if (thiz->isPause()) {
            thiz->wait();
        }
        if (thiz->isStop()) {
            break;
        }
        if (avFrame == nullptr) {
            continue;
        }

        double extra_delay = avFrame->repeat_pict / (2 * fps);
        double fps_delay = 1.0 / fps;

        double video_time = avFrame->best_effort_timestamp * av_q2d(timeBase);
        double audio_play_time = thiz->videoPlayerCallback->getAudioPlayTime();

        double real_delay = 0;
        if (audio_play_time > 0) {
            double time_diff = video_time - audio_play_time;
            if (time_diff > 0) {
                real_delay = (fps_delay + extra_delay + time_diff) * 1000000;
            } else if (time_diff < 0) {
                time_diff = fabs(time_diff);
                if (time_diff > 0.05) {
                    releaseAVFrame(&avFrame);
                    continue;
                }
            }
        }
        if (real_delay < 0) {
            real_delay = (fps_delay + extra_delay) * 1000000;
        }
        av_usleep((int) real_delay);

        sws_scale(swsContext,
                  avFrame->data, avFrame->linesize, 0, avFrame->height,
                  dst_data, dst_stride
        );
        thiz->renderVideo(dst_data[0], dstW, dstH, dst_stride[0]);
        releaseAVFrame(&avFrame);
    }

    releaseAVFrame(&avFrame);
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
    return nullptr;
}

void VideoChannel::pausePlay() {

}

void VideoChannel::stopPlay() {
    pthread_join(video_pid, nullptr);
    if (this->aNativeWindow) {
//        ANativeWindow_release(this->aNativeWindow);
        this->aNativeWindow = nullptr;
    }
}

void VideoChannel::renderVideo(uint8_t *srcData, int src_width, int src_height, int src_stride) {
    ANativeWindow *window = this->getWindow();
    int window_width = ANativeWindow_getWidth(window);
    int window_height = ANativeWindow_getHeight(window);
    ANativeWindow_setBuffersGeometry(window, window_width, window_height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, nullptr) != 0) {
        return;
    }

    uint8_t *dst_data = static_cast<uint8_t *>(buffer.bits);
    int dst_stride = buffer.stride * 4;

    uint8_t *fill_data[4];
    int fill_linesize[4];
    int fillWidth = 0;
    int fillHeight = 0;
    int fill_stride = 0;

    if (src_width == window_width) {
        fillWidth = window_width;
        fillHeight = (window_height - src_height) / 2;
    } else {
        fillWidth = (window_width - src_width) / 2;
        fillHeight = window_height;
        av_image_fill_linesizes(fill_linesize, AV_PIX_FMT_RGBA, fillWidth);

        fill_stride = fill_linesize[0];
        int offset = dst_stride - (fill_stride * 2) - src_stride;
        if (offset > 0) {
            fillWidth = fillWidth + offset / 2;
        }
    }

    av_image_alloc(fill_data, fill_linesize, fillWidth, fillHeight, AV_PIX_FMT_RGBA, 1);
    av_image_fill_black(fill_data, fill_linesize, AV_PIX_FMT_RGBA,
                        AVColorRange::AVCOL_RANGE_JPEG,
                        fillWidth, fillHeight);
    fill_stride = fill_linesize[0];

    if (src_width == window_width) {
        int bottomHeight = fillHeight + src_height + 1;
        for (int i = 0; i < buffer.height; ++i) {
            if (i <= fillHeight) {
                memcpy(dst_data + i * dst_stride, fill_data[0] + i * fill_linesize[0],
                       dst_stride);
                continue;
            }
            if (i >= bottomHeight) {
                int k = i - bottomHeight;
                memcpy(dst_data + i * dst_stride, fill_data[0] + k * fill_linesize[0],
                       dst_stride);
                continue;
            }

            int j = i - fillHeight;
            memcpy(dst_data + i * dst_stride, srcData + j * src_stride, dst_stride);
        }
    } else {
        for (int i = 0; i < buffer.height; ++i) {
            uint8_t *dst_data_data = dst_data + i * dst_stride;

            memcpy(dst_data_data, fill_data[0] + i * fill_linesize[0], fill_stride);

            dst_data_data = dst_data_data + fill_stride;
            memcpy(dst_data_data, srcData + i * src_stride, src_stride);

            dst_data_data = dst_data_data + src_stride;
            memcpy(dst_data_data, fill_data[0] + i * fill_linesize[0], fill_stride);
        }
    }

    ANativeWindow_unlockAndPost(window);
    av_freep(&fill_data[0]);
}

ANativeWindow *VideoChannel::getWindow() {
    return this->aNativeWindow;
}

VideoChannel::~VideoChannel() {
}

void VideoChannel::getAVPacket(AVPacket **value) {
    this->videoPlayerCallback->getVideoAVPacket(value);
}

