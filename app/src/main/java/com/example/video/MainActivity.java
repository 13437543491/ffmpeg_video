package com.example.video;


import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;

public class MainActivity extends Activity {

    static {
        System.loadLibrary("video");
    }

    private static final String sSoundPath = "file:///android_asset/derry.mp3";

    private SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main);
        mSurfaceView = findViewById(R.id.surfaceView);

        VideoImpl video = new VideoImpl();

        findViewById(R.id.btn_start).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                video.start("/storage/emulated/0/1111.mp4");
//                video.start("/storage/emulated/0/2222.mp4");
//                video.start("rtmp://ns8.indexforce.com/home/mystream");
//                video.start("https://cn-sxty-cu-03-03.bilivideo.com/upgcxcode/85/41/794214185/794214185_da2-1-16.mp4?e=ig8euxZM2rNcNbRVhwdVhwdlhWdVhwdVhoNvNC8BqJIzNbfq9rVEuxTEnE8L5F6VnEsSTx0vkX8fqJeYTj_lta53NCM=&uipk=5&nbs=1&deadline=1703094199&gen=playurlv2&os=bcache&oi=2018887345&trid=0000df10c6e5406e4980812895fc110986c8h&mid=339709080&platform=html5&upsig=8570ddf1041bf988503c40acce9ebb3f&uparams=e,uipk,nbs,deadline,gen,os,oi,trid,mid,platform&cdnid=71403&bvc=vod&nettype=0&f=h_0_0&bw=36180&logo=80000000");
//                video.start("http://vjs.zencdn.net/v/oceans.mp4");
            }
        });

        findViewById(R.id.btn_pause).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                video.pause();
            }
        });

        findViewById(R.id.btn_stop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                video.stop();
            }
        });

        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {

            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.e("yangyangyang", "surfaceChanged");
                video.setmSurface(holder.getSurface());

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });

        Log.e("yangyangyang", "111111111");
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}