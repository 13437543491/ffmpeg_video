package com.example.video;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.widget.VideoView;

public class MainActivity2 extends Activity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main2);

        VideoView videoView = findViewById(R.id.videoView);
        videoView.setVideoPath("/storage/emulated/0/1111.mp4");
        videoView.start();

    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}