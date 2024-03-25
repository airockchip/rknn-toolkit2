package com.rockchip.gpadc.demo;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.graphics.Typeface;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.rockchip.gpadc.demo.rga.RGA;
import com.rockchip.gpadc.demo.yolo.InferenceWrapper;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;

import static com.rockchip.gpadc.demo.rga.HALDefine.CAMERA_PREVIEW_HEIGHT;
import static com.rockchip.gpadc.demo.rga.HALDefine.CAMERA_PREVIEW_WIDTH;
import static com.rockchip.gpadc.demo.rga.HALDefine.IM_HAL_TRANSFORM_FLIP_H;
import static com.rockchip.gpadc.demo.rga.HALDefine.RK_FORMAT_RGBA_8888;
import static com.rockchip.gpadc.demo.rga.HALDefine.RK_FORMAT_YCrCb_420_SP;
import static com.rockchip.gpadc.demo.yolo.PostProcess.INPUT_CHANNEL;
import static java.lang.Thread.sleep;


public class CameraPreviewActivity extends Activity implements Camera.PreviewCallback {

    private final String TAG = "rkyolo";
    private static final int MAGIC_TEXTURE_ID = 10;

    TSurfaceHolderCallback mSurfaceHolderCallback = null;

    private Camera mCamera0 = null;
    private SurfaceView mSurfaceView = null;
    public SurfaceTexture mSurfaceTexture = null;
    private SurfaceHolder mSurfaceHolder = null;
    public int flip = -1;    // for CAMERA_FACING_BACK(camera comes with RK3588 using this mode),
                             // we do not need flip, using -1, or we need using
                             // IM_HAL_TRANSFORM_FLIP_H

    private boolean mIsCameraOpened = false;
    private int mCameraId = -1;
    public byte textureBuffer[];

    // for inference
    private String mModelName = "yolov5s.rknn";
    private String platform = "rk3588";
    private InferenceWrapper mInferenceWrapper;
    private String fileDirPath;     // file dir to store model cache
    private ImageBufferQueue mImageBufferQueue;    // intermedia between camera thread and  inference thread
    private InferenceResult mInferenceResult = new InferenceResult();  // detection result
    private int mWidth;    //surface width
    private int mHeight;    //surface height
    private volatile boolean mStopInference = false;

    //draw result
    private TextView mFpsNum1;
    private TextView mFpsNum2;
    private TextView mFpsNum3;
    private TextView mFpsNum4;
    private ImageView mTrackResultView;
    private Bitmap mTrackResultBitmap = null;
    private Canvas mTrackResultCanvas = null;
    private Paint mTrackResultPaint = null;
    private Paint mTrackResultTextPaint = null;

    private PorterDuffXfermode mPorterDuffXfermodeClear;
    private PorterDuffXfermode mPorterDuffXfermodeSRC;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        mFpsNum1 = (TextView) findViewById(R.id.fps_num1);
        mFpsNum2 = (TextView) findViewById(R.id.fps_num2);
        mFpsNum3 = (TextView) findViewById(R.id.fps_num3);
        mFpsNum4 = (TextView) findViewById(R.id.fps_num4);
        mTrackResultView = (ImageView) findViewById(R.id.canvasView);

        fileDirPath = getCacheDir().getAbsolutePath();

        platform = getPlatform();
        Log.d(TAG, "get soc platform:" + platform);

        if (platform.equals("rk3588")) {
            createFile(mModelName, R.raw.yolov5s_rk3588);
        } else if (platform.equals("rk356x")) {
            createFile(mModelName, R.raw.yolov5s_rk3566);
        } else if (platform.equals("rk3562")) {
            createFile(mModelName, R.raw.yolov5s_rk3562);
        } else if (platform.equals("rk3576")) {
            createFile(mModelName, R.raw.yolov5s_rk3576);
        } else {
            Toast toast = Toast.makeText(this, "Can not get platform use RK3588 instead.", Toast.LENGTH_LONG);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
            createFile(mModelName, R.raw.yolov5s_rk3588);
        }

        try {
            mInferenceResult.init(getAssets());
        } catch (IOException e) {
            e.printStackTrace();
        }

        mInferenceWrapper = new InferenceWrapper();

    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");

        destroyPreviewView();
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        stopTrack();
        stopCamera();
        destroyPreviewView();
        super.onPause();

    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");

        createPreviewView();
        super.onResume();

    }

    private boolean createPreviewView() {
        mSurfaceView = findViewById(R.id.surfaceViewCamera1);
        mSurfaceHolder = mSurfaceView.getHolder();
//        mSurfaceView.setZOrderMediaOverlay(true);

        mSurfaceTexture = new SurfaceTexture(MAGIC_TEXTURE_ID);

        mSurfaceHolderCallback = new TSurfaceHolderCallback();
        mSurfaceHolder.addCallback(mSurfaceHolderCallback);

        return true;
    }

    private void destroyPreviewView() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.removeCallback(mSurfaceHolderCallback);
            mSurfaceHolderCallback = null;
            mSurfaceHolder = null;
        }

    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        mCamera0.addCallbackBuffer(data);
        ImageBufferQueue.ImageBuffer imageBuffer = mImageBufferQueue.getFreeBuffer();

        if (imageBuffer != null) {
            // RK_FORMAT_YCrCb_420_SP -> RK_FORMAT_RGBA_8888
            // flip for CAMERA_FACING_FRONT
            RGA.colorConvertAndFlip(data, RK_FORMAT_YCrCb_420_SP,
                    imageBuffer.mImage, RK_FORMAT_RGBA_8888,
                    CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT, this.flip);

            mImageBufferQueue.postBuffer(imageBuffer);
        }
    }

    private class TSurfaceHolderCallback implements SurfaceHolder.Callback {

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            Log.d(TAG, "surfaceChanged");
            mWidth = width;
            mHeight = height;

            textureBuffer=new byte[CAMERA_PREVIEW_WIDTH * CAMERA_PREVIEW_HEIGHT * 4];
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            Log.d(TAG, "surfaceCreated");

            startCamera();
            startTrack();

        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            Log.d(TAG, "surfaceDestroyed");

            stopTrack();
            stopCamera();
        }
    }


    private boolean startCamera() {
        if (mIsCameraOpened) {
            return true;
        }

        //(Camera.CameraInfo.CAMERA_FACING_BACK);
        int num = Camera.getNumberOfCameras();
        if (num > 2)
            mCameraId = 2;
        else
            mCameraId = 0;
        Log.d(TAG, "mCameraId = " + mCameraId);
        Camera.CameraInfo camInfo = new Camera.CameraInfo();
        try {
            Camera.getCameraInfo(mCameraId, camInfo);
            if (mCameraId != -1) {
                mCamera0 = Camera.open(mCameraId);
            } else {
                mCamera0 = Camera.open();
            }
            Log.d(TAG, "mCamera0 = " + mCamera0);
            Log.d(TAG, "camera facing: " + camInfo.facing);
            if (Camera.CameraInfo.CAMERA_FACING_FRONT == camInfo.facing) {
                this.flip = IM_HAL_TRANSFORM_FLIP_H;
            }

        } catch (RuntimeException e) {
            Log.w(TAG, "Unable to open camera!");
            Toast toast = Toast.makeText(this, "Unable to open camera!", Toast.LENGTH_LONG);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
            return false;
        }

        setCameraParameters();

        try {
            mCamera0.setPreviewDisplay(mSurfaceHolder);
            mCamera0.setDisplayOrientation(0);
            int BUFFER_SIZE0 = CAMERA_PREVIEW_WIDTH * CAMERA_PREVIEW_HEIGHT * 3 / 2; // NV21
            byte[][] mPreviewData0 = new byte[][]{new byte[BUFFER_SIZE0], new byte[BUFFER_SIZE0],new byte[BUFFER_SIZE0]};
            //================================
            for (byte[] buffer : mPreviewData0)
                mCamera0.addCallbackBuffer(buffer);
            mCamera0.setPreviewCallbackWithBuffer(this);
            //==================================
            mCamera0.startPreview();
        } catch (Exception e) {
            mCamera0.release();
            return false;
        }

        mIsCameraOpened = true;

        return true;
    }

    private void stopCamera() {
        if (mIsCameraOpened) {
            mCamera0.setPreviewCallback(null);
            mCamera0.stopPreview();
            mCamera0.release();
            mCamera0 = null;
            mIsCameraOpened = false;
        }

    }

    private void setCameraParameters() {
        Camera.Parameters parameters;
        boolean checkWH = false;
        parameters = mCamera0.getParameters();
        int nearest_width_index = 0;
        int nearest_width_value = 1920;

        // TODO, android sdk > android 14
        // mCamera0.setParameters(parameters); maybe fail under android 14
        if (Build.VERSION.SDK_INT >=34) {
            Camera.Size size = parameters.getPreviewSize();
            CAMERA_PREVIEW_WIDTH = size.width;
            CAMERA_PREVIEW_HEIGHT = size.height;
            Log.i(TAG, "Use preview Size = " + CAMERA_PREVIEW_WIDTH + "x" + CAMERA_PREVIEW_HEIGHT);
            return;
        }

        List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();
        for (int i = 0; i < sizes.size(); i++) {
            Camera.Size size = sizes.get(i);

            if (Math.abs(size.width-CAMERA_PREVIEW_WIDTH) < nearest_width_value ) {
                nearest_width_value = Math.abs(size.width-CAMERA_PREVIEW_WIDTH);
                nearest_width_index = i;
            }

            if ( (size.width == CAMERA_PREVIEW_WIDTH) && (size.height == CAMERA_PREVIEW_HEIGHT)) {
                checkWH = true;
            }

            Log.v(TAG, "Camera Supported Preview Size = " + size.width + "x" + size.height);
        }
        if (!checkWH) {
            Log.e(TAG, "Camera don't support this preview Size = " + CAMERA_PREVIEW_WIDTH + "x" + CAMERA_PREVIEW_HEIGHT);
            CAMERA_PREVIEW_WIDTH = sizes.get(nearest_width_index).width;
            CAMERA_PREVIEW_HEIGHT = sizes.get(nearest_width_index).height;
        }

        Log.i(TAG, "Use preview Size = " + CAMERA_PREVIEW_WIDTH + "x" + CAMERA_PREVIEW_HEIGHT);

        parameters.setPreviewSize(CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT);

        if (parameters.isZoomSupported()) {
            parameters.setZoom(0);
        }
        mCamera0.setParameters(parameters);
        Log.i(TAG, "mCamera0 set parameters success.");
    }

    private void startTrack() {
        mInferenceResult.reset();
        mImageBufferQueue = new ImageBufferQueue(3, CAMERA_PREVIEW_WIDTH, CAMERA_PREVIEW_HEIGHT);
        mStopInference = false;
        mInferenceThread = new Thread(mInferenceRunnable);
        mInferenceThread.start();
    }

    private void stopTrack() {

        mStopInference = true;
        try {
            if (mInferenceThread != null) {
                mInferenceThread.join();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        if (mImageBufferQueue != null) {
            mImageBufferQueue.release();
            mImageBufferQueue = null;
        }
    }

    private Thread mInferenceThread;
    private Runnable mInferenceRunnable = new Runnable() {
        public void run() {

            int count = 0;
            long oldTime = System.currentTimeMillis();
            long currentTime;

            updateMainUI(1, 0);

            String paramPath = fileDirPath + "/" + mModelName;

            try {
                mInferenceWrapper.initModel(CAMERA_PREVIEW_HEIGHT, CAMERA_PREVIEW_WIDTH, INPUT_CHANNEL, paramPath);
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(1);
            }


            while (!mStopInference) {
                ImageBufferQueue.ImageBuffer buffer = mImageBufferQueue.getReadyBuffer();

                if (buffer == null) {
                    try {
//                        Log.w(TAG, "buffer is null.");
                        sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    continue;
                }

                InferenceResult.OutputBuffer outputs = mInferenceWrapper.run(buffer.mImage);

                mInferenceResult.setResult(outputs);

                mImageBufferQueue.releaseBuffer(buffer);

                if (++count >= 30) {
                    currentTime = System.currentTimeMillis();

                    float fps = count * 1000.f / (currentTime - oldTime);

//                    Log.d(TAG, "current fps = " + fps);

                    oldTime = currentTime;
                    count = 0;
                    updateMainUI(0, fps);

                }

                updateMainUI(1, 0);
            }

            mInferenceWrapper.deinit();
        }
    };

    private void createFile(String fileName, int id) {
        String filePath = fileDirPath + "/" + fileName;
        try {
            File dir = new File(fileDirPath);

            if (!dir.exists()) {
                dir.mkdirs();
            }

            // 目录存在，则将apk中raw中的需要的文档复制到该目录下
            File file = new File(filePath);

            if (!file.exists() || isFirstRun()) {

                InputStream ins = getResources().openRawResource(id);// 通过raw得到数据资源
                FileOutputStream fos = new FileOutputStream(file);
                byte[] buffer = new byte[8192];
                int count = 0;

                while ((count = ins.read(buffer)) > 0) {
                    fos.write(buffer, 0, count);
                }

                fos.close();
                ins.close();

                Log.d(TAG, "Create " + filePath);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private boolean isFirstRun() {
        SharedPreferences sharedPreferences = getSharedPreferences("setting", MODE_PRIVATE);
        boolean isFirstRun = sharedPreferences.getBoolean("isFirstRun", true);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        if (isFirstRun) {
            editor.putBoolean("isFirstRun", false);
            editor.commit();
        }

        return isFirstRun;
    }

    // UI线程，用于更新处理结果
    private Handler mHandler = new Handler()
    {
        public void handleMessage(Message msg)
        {
            if (msg.what == 0) {
                float fps = (float) msg.obj;

                DecimalFormat decimalFormat = new DecimalFormat("00.00");
                String fpsStr = decimalFormat.format(fps);
                mFpsNum1.setText(String.valueOf(fpsStr.charAt(0)));
                mFpsNum2.setText(String.valueOf(fpsStr.charAt(1)));
                mFpsNum3.setText(String.valueOf(fpsStr.charAt(3)));
                mFpsNum4.setText(String.valueOf(fpsStr.charAt(4)));
            } else {
                showTrackSelectResults();
            }
        }
    };

    private void updateMainUI(int type, Object data) {
        Message msg = mHandler.obtainMessage();
        msg.what = type;
        msg.obj = data;
        mHandler.sendMessage(msg);
    }

    public static int sp2px(float spValue) {
        Resources r = Resources.getSystem();
        final float scale = r.getDisplayMetrics().scaledDensity;
        return (int) (spValue * scale + 0.5f);
    }

    private void showTrackSelectResults() {

        int width = CAMERA_PREVIEW_WIDTH;
        int height = CAMERA_PREVIEW_HEIGHT;

        if (mTrackResultBitmap == null) {

            mTrackResultBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            mTrackResultCanvas = new Canvas(mTrackResultBitmap);

            //用于画线
            mTrackResultPaint = new Paint();
            mTrackResultPaint.setColor(0xff06ebff);
            mTrackResultPaint.setStrokeJoin(Paint.Join.ROUND);
            mTrackResultPaint.setStrokeCap(Paint.Cap.ROUND);
            mTrackResultPaint.setStrokeWidth(4);
            mTrackResultPaint.setStyle(Paint.Style.STROKE);
            mTrackResultPaint.setTextAlign(Paint.Align.LEFT);
            mTrackResultPaint.setTextSize(sp2px(10));
            mTrackResultPaint.setTypeface(Typeface.SANS_SERIF);
            mTrackResultPaint.setFakeBoldText(false);

            //用于文字
            mTrackResultTextPaint = new Paint();
            mTrackResultTextPaint.setColor(0xff06ebff);
            mTrackResultTextPaint.setStrokeWidth(2);
            mTrackResultTextPaint.setTextAlign(Paint.Align.LEFT);
            mTrackResultTextPaint.setTextSize(sp2px(12));
            mTrackResultTextPaint.setTypeface(Typeface.SANS_SERIF);
            mTrackResultTextPaint.setFakeBoldText(false);


            mPorterDuffXfermodeClear = new PorterDuffXfermode(PorterDuff.Mode.CLEAR);
            mPorterDuffXfermodeSRC = new PorterDuffXfermode(PorterDuff.Mode.SRC);
        }

        // clear canvas
        mTrackResultPaint.setXfermode(mPorterDuffXfermodeClear);
        mTrackResultCanvas.drawPaint(mTrackResultPaint);
        mTrackResultPaint.setXfermode(mPorterDuffXfermodeSRC);

        //detect result
        ArrayList<InferenceResult.Recognition> recognitions = mInferenceResult.getResult(mInferenceWrapper);
        for (int i=0; i<recognitions.size(); ++i) {
            InferenceResult.Recognition rego = recognitions.get(i);
            RectF detection = rego.getLocation();

            detection.left *= width;
            detection.right *= width;
            detection.top *= height;
            detection.bottom *= height;

//            Log.d(TAG, rego.toString());
//            Log.d(TAG, detection.toString());

            mTrackResultCanvas.drawRect(detection, mTrackResultPaint);
            mTrackResultCanvas.drawText(rego.getTrackId() + " - " + mInferenceResult.mPostProcess.getLabelTitle(rego.getId()),
                    detection.left+5, detection.bottom-5, mTrackResultTextPaint);
        }

        mTrackResultView.setScaleType(ImageView.ScaleType.FIT_XY);
        mTrackResultView.setImageBitmap(mTrackResultBitmap);
    }

    private String getPlatform()//取平台版本
    {
        String platform = null;
        try {
            Class<?> classType = Class.forName("android.os.SystemProperties");
            Method getMethod = classType.getDeclaredMethod("get", new Class<?>[]{String.class});
            platform = (String) getMethod.invoke(classType, new Object[]{"ro.board.platform"});
        } catch (Exception e) {
            e.printStackTrace();
        }
        return platform;
    }
}
