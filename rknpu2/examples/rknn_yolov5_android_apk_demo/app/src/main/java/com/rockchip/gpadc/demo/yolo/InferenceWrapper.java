package com.rockchip.gpadc.demo.yolo;

import android.graphics.RectF;
import android.util.Log;

import com.rockchip.gpadc.demo.InferenceResult;
import com.rockchip.gpadc.demo.InferenceResult.OutputBuffer;
import com.rockchip.gpadc.demo.InferenceResult.Recognition;
import com.rockchip.gpadc.demo.InferenceResult.DetectResultGroup;

import java.io.IOException;
import java.util.ArrayList;

/**
 * Created by randall on 18-4-18.
 */

public class InferenceWrapper {
    private final String TAG = "rkyolo.InferenceWrapper";

    static {
        System.loadLibrary("rknn4j");
    }

    OutputBuffer mOutputs;
    ArrayList<Recognition> mRecognitions = new ArrayList<Recognition>();
    DetectResultGroup mDetectResults;

    public int OBJ_NUMB_MAX_SIZE = 64;
//    public int inf_count = 0;
//    public int post_count = 0;
//    public long inf_time = 0;
//    public long post_time = 0;


    public InferenceWrapper() {

    }

    public int initModel(int im_height, int im_width, int im_channel, String modelPath) throws Exception {
        mOutputs = new InferenceResult.OutputBuffer();
        mOutputs.mGrid0Out = new byte[255 * 80 * 80];
        mOutputs.mGrid1Out = new byte[255 * 40 * 40];
        mOutputs.mGrid2Out = new byte[255 * 20 * 20];
        if (navite_init(im_height, im_width, im_channel, modelPath) != 0) {
            throw new IOException("rknn init fail!");
        }
        return 0; 
    }


    public void deinit() {
        native_deinit();
        mOutputs.mGrid0Out = null;
        mOutputs.mGrid1Out = null;
        mOutputs.mGrid2Out = null;
        mOutputs = null;

    }

    public InferenceResult.OutputBuffer run(long img_buf_handle, int camera_width, int camera_height) {
//        long startTime = System.currentTimeMillis();
//        long endTime;
        native_run(img_buf_handle, camera_width, camera_height, mOutputs.mGrid0Out, mOutputs.mGrid1Out, mOutputs.mGrid2Out);
//        this.inf_count += 1;
//        endTime = System.currentTimeMillis();
//        this.inf_time += (endTime - startTime);
//        if (this.inf_count >= 100) {
//            float inf_avg = this.inf_time * 1.0f / this.inf_count;
//            Log.w(TAG, String.format("inference avg cost: %.5f ms", inf_avg));
//            this.inf_count = 0;
//            this.inf_time = 0;
//        }
//        Log.i(TAG, String.format("inference count: %d", this.inf_count));
        return  mOutputs;
    }

    public ArrayList<InferenceResult.Recognition> postProcess(InferenceResult.OutputBuffer outputs) {
        ArrayList<Recognition> recognitions = new ArrayList<Recognition>();

        mDetectResults = new DetectResultGroup();
        mDetectResults.count = 0;
        mDetectResults.ids = new int[OBJ_NUMB_MAX_SIZE];
        mDetectResults.scores = new float[OBJ_NUMB_MAX_SIZE];
        mDetectResults.boxes = new float[4 * OBJ_NUMB_MAX_SIZE];

        if (null == outputs || null == outputs.mGrid0Out || null == outputs.mGrid1Out
                || null == outputs.mGrid2Out) {
            return recognitions;
        }

//        long startTime = System.currentTimeMillis();
//        long endTime;
        int count = native_post_process(outputs.mGrid0Out, outputs.mGrid1Out, outputs.mGrid2Out,
                mDetectResults.ids, mDetectResults.scores, mDetectResults.boxes);
        if (count < 0) {
            Log.w(TAG, "post_process may fail.");
            mDetectResults.count = 0;
        } else {
            mDetectResults.count = count;
        }
//        Log.i(TAG, String.format("Detected %d objects", count));
//        this.post_count += 1;
//        Log.i(TAG, String.format("post count: %d", this.post_count));

        for (int i = 0; i < count; ++i) {
            RectF rect = new RectF();
            rect.left = mDetectResults.boxes[i*4+0];
            rect.top = mDetectResults.boxes[i*4+1];
            rect.right = mDetectResults.boxes[i*4+2];
            rect.bottom = mDetectResults.boxes[i*4+3];

            Recognition recog = new InferenceResult.Recognition(mDetectResults.ids[i],
                    mDetectResults.scores[i], rect);
            recognitions.add(recog);
        }
//        endTime = System.currentTimeMillis();
//        this.post_time += (endTime - startTime);
//        if (this.post_count >= 100) {
//            float post_avg = this.post_time * 1.0f / this.post_count;
//            Log.w(TAG, String.format("post process avg cost: %.5f ms", post_avg));
//            this.post_time = 0;
//            this.post_count = 0;
//        }

        return recognitions;
    }

    private native int navite_init(int im_height, int im_width, int im_channel, String modelPath);
    private native void native_deinit();
    private native int native_run(long img_buf_handle, int cam_width, int cam_height, byte[] grid0Out, byte[] grid1Out, byte[] grid2Out);
    private native int native_post_process(byte[] grid0Out, byte[] grid1Out, byte[] grid2Out,
                                           int[] ids, float[] scores, float[] boxes);

}