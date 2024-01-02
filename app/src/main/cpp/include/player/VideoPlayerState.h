#ifndef FFMPEGVIDEO_VIDEOPLAYERSTATE_H
#define FFMPEGVIDEO_VIDEOPLAYERSTATE_H

#define STATE_START 1
#define STATE_PAUSE 2
#define STATE_STOP 3
#define STATE_DESTROY 4

class VideoPlayerState {
public:
    virtual bool isPlaying() = 0;

    virtual bool isPause() = 0;

    virtual bool isStop() = 0;

    virtual bool isDestroy() = 0;

};

#endif //FFMPEGVIDEO_VIDEOPLAYERSTATE_H
