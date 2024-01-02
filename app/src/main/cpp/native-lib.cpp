#include <jni.h>
#include "player/VideoPlayerImpl.h"

VideoPlayerImpl *player = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_video_VideoImpl_nativeStart(JNIEnv *env, jobject thiz, jstring path,
                                             jobject surface) {
    const char *path_ = env->GetStringUTFChars(path, nullptr);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (player == nullptr) {
        player = new VideoPlayerImpl();
    }

    player->playVideo(path_, window);

    env->ReleaseStringUTFChars(path, path_);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_video_VideoImpl_nativePause(JNIEnv *env, jobject thiz) {
    if (player) {
        player->pause();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_video_VideoImpl_nativeStop(JNIEnv *env, jobject thiz) {
    if (player) {
        player->stop();
    }
}