package com.example.client;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.Vibrator;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Button;
import android.widget.CheckBox;
import android.os.Handler;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private Handler handler = new Handler();

    private float g_strong;

    private SeekBar seekBar, seekBar2, seekBar3, seekBar4, seekBar5;
    private CheckBox bEsea, bHs;

    private Runnable runnable = new Runnable() {
        int previous_tick = 0;
        private void update()
        {

            int status = getBestTarget(bEsea.isChecked() ? 1 : 0);

            if (status == -1) {
                resetTarget();
                findViewById(R.id.main_layout).setVisibility(View.GONE);
                findViewById(R.id.ip_layout).setVisibility(View.VISIBLE);
                handler.removeCallbacks(runnable);
                closeClient();
            }

            if (status == 0) {
                resetTarget();
                previous_tick = 0;
                return;
            }

            if (getPlayerInformation() == 0) {
                resetTarget();
                return;
            }

            int def = targetDefusing();

            if (hasTarget() == 1) {
                if (getCurrentTick() - previous_tick > 14) {
                    previous_tick = getCurrentTick();
                    //((Vibrator)getSystemService(Context.VIBRATOR_SERVICE)).
                    //        vibrate(150);
                    Beep(50, seekBar3.getProgress(), ToneGenerator.TONE_CDMA_ANSWER);
                }
            }
            if (def == 1) {
                if (getCurrentTick() - previous_tick > 14) {
                    previous_tick = getCurrentTick();

                    Beep(50, seekBar3.getProgress(), ToneGenerator.TONE_CDMA_CONFIRM);
                }
            }
            else if (def == 2) {
                if (getCurrentTick() - previous_tick > 14) {
                    previous_tick = getCurrentTick();

                    Beep(50, seekBar3.getProgress(), ToneGenerator.TONE_CDMA_EMERGENCY_RINGBACK);
                }
            }

            g_strong = 31.0f - (float)seekBar.getProgress() / 100.0f;
            aimAtTarget(
                    getTargetAngle(),
                    (float)seekBar2.getProgress() / 100.0f,
                    g_strong,
                    (float)seekBar4.getProgress() / 10.0f,
                    (float)seekBar5.getProgress() / 10.0f,
                    bHs.isChecked() ? 1 : 0);
        }

        @Override
        public void run() {
            update();
            handler.postDelayed(runnable, 1);
        }
    } ;



    private void Beep(int duration, int volume, int sound)
    {
        final ToneGenerator tone = new ToneGenerator(AudioManager.STREAM_MUSIC, volume);
        tone.startTone(sound, duration);
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                if (tone != null)
                tone.release();
            }
        }, duration + 50);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        seekBar = ((SeekBar)findViewById(R.id.aim_strong));
        seekBar2 = ((SeekBar)findViewById(R.id.aim_fov));
        seekBar3 = ((SeekBar)findViewById(R.id.volume));
        seekBar4 = ((SeekBar)findViewById(R.id.preaim));
        seekBar5 = ((SeekBar)findViewById(R.id.sensitivity));
        bEsea = ((CheckBox)findViewById(R.id.incross));
        bHs = ((CheckBox)findViewById(R.id.hs));



        final TextView fov_text = (TextView)findViewById(R.id.fov_text);
        final TextView strength_text = (TextView)findViewById(R.id.strength);
        final TextView volume = (TextView)findViewById(R.id.volume_text);
        final TextView preaim = (TextView)findViewById(R.id.preaim_text);
        final TextView sensitivity = (TextView)findViewById(R.id.sensitivity_text);

        strength_text.setText("Strength: " +
                Float.toString((float)seekBar.getProgress() / 100.0f));

        fov_text.setText("Fov: " +
                (Float.toString((float)seekBar2.getProgress() / 100.0f)));

        volume.setText("Volume: " +
                Integer.toString(seekBar3.getProgress()));

        preaim.setText("PreAim: " +
                Float.toString((float)seekBar4.getProgress() / 10.0f));

        sensitivity.setText("Sensitivity: " +
                Float.toString((float)seekBar5.getProgress() / 10.0f));


        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                strength_text.setText("Strength: " +
                        Float.toString((float)progress / 100.0f));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBar2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                fov_text.setText("Fov: " +
                        (Float.toString((float)progress / 100.0f)));
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBar3.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                volume.setText("Volume: " +
                        Integer.toString(progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBar4.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                preaim.setText("PreAim: " +
                        Float.toString((float)progress / 10.0f));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        seekBar5.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                sensitivity.setText("Sensitivity: " +
                        Float.toString((float)progress / 10.0f));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        g_strong = 31.0f - (float)seekBar.getProgress() / 100.0f;


        // Example of a call to a native method

        final Button connectButton = findViewById(R.id.connect_button);


        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText ipText = findViewById(R.id.ip_text);
                if (initClient(ipText.getText().toString()) != 0)
                    return;

                findViewById(R.id.ip_layout).setVisibility(View.GONE);
                findViewById(R.id.main_layout).setVisibility(View.VISIBLE);
                handler.post(runnable);
            }

        });
    }

    /*
    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        handler.removeCallbacks(runnable);
        destroyClient();
    }*/

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int initClient(String ip);
    public native void closeClient();
    public native int getBestTarget(int besea);
    public native void resetTarget();
    public native int hasTarget();
    public native int targetDefusing();
    public native int getCurrentTick();
    public native int getPlayerInformation();
    public native float[] getTargetAngle();
    public native void aimAtTarget(float[] angle, float fov, float smooth,
                                   float preaim, float sensitivity, int hs);
}
