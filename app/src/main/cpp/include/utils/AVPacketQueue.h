#ifndef FFMPEGVIDEO_AVPACKETQUEUE_H
#define FFMPEGVIDEO_AVPACKETQUEUE_H

#include <pthread.h>
#include <queue>
#include "release_util.h"

extern "C" {
#include "ffmpeg/libavcodec/avcodec.h"
};

class AVPacketQueue {
private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<AVPacket *> queue;
    bool isDestroy = false;

public:
    AVPacketQueue() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~AVPacketQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void init() {
        pthread_mutex_lock(&mutex);
        isDestroy = false;
        pthread_mutex_unlock(&mutex);
    }

    void push(AVPacket *value) {
        pthread_mutex_lock(&mutex);
        if (!isDestroy) {
            queue.push(value);
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }

    void get(AVPacket **value) {
        pthread_mutex_lock(&mutex);

        if (!isDestroy) {
            if (queue.empty()) {
                pthread_cond_wait(&cond, &mutex);
            }
            if (!isDestroy) {
                *value = queue.front();
                queue.pop();
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    void destroy() {
        pthread_mutex_lock(&mutex);
        isDestroy = true;
        pthread_cond_broadcast(&cond);
        while (!queue.empty()) {
            AVPacket *value = queue.front();
            queue.pop();
            releaseAVPacket(&value);
        }
        pthread_mutex_unlock(&mutex);
    }
};

#endif //FFMPEGVIDEO_AVPACKETQUEUE_H
