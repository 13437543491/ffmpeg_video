#ifndef FFMPEGVIDEO_AVFRAMEQUEUE_H
#define FFMPEGVIDEO_AVFRAMEQUEUE_H

#include <pthread.h>
#include <queue>
#include "utils/LogUtil.h"
#include "release_util.h"

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
};

class AVFrameQueue {
private:
    static const int max_size = 16;

    pthread_mutex_t mutex;

    pthread_cond_t cond;

    std::queue<AVFrame *> queue;

    bool isDestroy = false;

public:
    AVFrameQueue() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~AVFrameQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void init() {
        pthread_mutex_lock(&mutex);
        isDestroy = false;
        pthread_mutex_unlock(&mutex);
    }

    void push(AVFrame *src_frame) {
        pthread_mutex_lock(&mutex);
        if (!isDestroy) {
            while (queue.size() >= max_size && !isDestroy) {
                pthread_cond_wait(&cond, &mutex);
            }
            if (!isDestroy) {
                queue.push(src_frame);
                pthread_cond_signal(&cond);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    void get(AVFrame **value) {
        pthread_mutex_lock(&mutex);
        if (!isDestroy) {
            if (queue.empty()) {
                pthread_cond_wait(&cond, &mutex);
            }

            if (!isDestroy) {
                *value = queue.front();
                queue.pop();
                pthread_cond_signal(&cond);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    void destroy() {
        pthread_mutex_lock(&mutex);
        isDestroy = true;
        pthread_cond_broadcast(&cond);
        while (!queue.empty()) {
            AVFrame *value = queue.front();
            queue.pop();
            releaseAVFrame(&value);
        }
        pthread_mutex_unlock(&mutex);
    }
};

#endif //FFMPEGVIDEO_AVFRAMEQUEUE_H
