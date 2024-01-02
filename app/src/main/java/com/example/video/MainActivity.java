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
                video.start("http://video19.ifeng.com/video09/2019/09/17/p24510076-102-009-123825.mp4?reqtype=tsl&vkey=uT%2FAgfr2R0wz548u572M0icOGveiZRi70GgFgO%2Fs%2F9sTZzclBLOsk0bcB4ndENEHQCJnFZPpV1uwALuJhsiNR7pgZLmgCOJIu04fZjcJ7VPPotv6Dtmv1tTT6ZQqCsIu%2F6%2Bn3cOlfMFrGxJEHQCHt1A6mgPCkDREdRGnaQBvJyOmAXVWLyiznnXDPNqrVEYWdsYBsVFqo1xR8IOkVf59UQ%3D%3D");
            }
        });

        findViewById(R.id.btn_pause).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                video.pause();
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