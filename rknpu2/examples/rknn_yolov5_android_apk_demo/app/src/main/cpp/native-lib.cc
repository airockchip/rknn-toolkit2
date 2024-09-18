
#include <jni.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>

#include <sched.h>

#include "yolo_image.h"
#include "rga/rga.h"
#include "object_tracker/track_link.h"

static char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);

    if (alen > 0) {
        rtn = new char[alen + 1];
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

extern "C"
JNIEXPORT jint JNICALL Java_com_rockchip_gpadc_demo_yolo_InferenceWrapper_navite_1init
  (JNIEnv *env, jobject obj, jint im_height, jint im_width, jint im_channel,
   jstring model_path)
{
	char *model_path_p = jstringToChar(env, model_path);
	return create(im_height, im_width, im_channel, model_path_p);
}

extern "C"
JNIEXPORT void JNICALL Java_com_rockchip_gpadc_demo_yolo_InferenceWrapper_native_1deinit
		(JNIEnv *env, jobject obj) {
	destroy();

}

extern "C"
JNIEXPORT jint JNICALL Java_com_rockchip_gpadc_demo_yolo_InferenceWrapper_native_1run
  (JNIEnv *env, jobject obj, jlong img_buf_handle, 
	jint cam_width, jint cam_height,
   jbyteArray grid0Out, jbyteArray grid1Out, jbyteArray grid2Out) {

  	jboolean inputCopy = JNI_FALSE;
  	// jbyte* const inData = env->GetByteArrayElements(in, &inputCopy);

 	jboolean outputCopy = JNI_FALSE;

  	jbyte* const y0 = env->GetByteArrayElements(grid0Out, &outputCopy);
	jbyte* const y1 = env->GetByteArrayElements(grid1Out, &outputCopy);
	jbyte* const y2 = env->GetByteArrayElements(grid2Out, &outputCopy);

	run_yolo(img_buf_handle, cam_width, cam_height, (char *)y0, (char *)y1, (char *)y2);

	//env->ReleaseByteArrayElements(in, inData, JNI_ABORT);
	env->ReleaseByteArrayElements(grid0Out, y0, 0);
	env->ReleaseByteArrayElements(grid1Out, y1, 0);
	env->ReleaseByteArrayElements(grid2Out, y2, 0);

	return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchip_gpadc_demo_yolo_InferenceWrapper_native_1post_1process(JNIEnv *env, jobject thiz,
																		 jbyteArray grid0_out,
																		 jbyteArray grid1_out,
																		 jbyteArray grid2_out,
																		 jintArray ids,
																		 jfloatArray scores,
																		 jfloatArray boxes) {
	jint detect_counts;
	jboolean inputCopy = JNI_FALSE;
	jbyte* const grid0_buf = env->GetByteArrayElements(grid0_out, &inputCopy);
	jbyte* const grid1_buf = env->GetByteArrayElements(grid1_out, &inputCopy);
	jbyte* const grid2_buf = env->GetByteArrayElements(grid2_out, &inputCopy);

	jboolean outputCopy = JNI_FALSE;

	jint*   const y0 = env->GetIntArrayElements(ids, &outputCopy);
	jfloat* const y1 = env->GetFloatArrayElements(scores, &outputCopy);
	jfloat* const y2 = env->GetFloatArrayElements(boxes, &outputCopy);

	detect_counts = yolo_post_process((char *)grid0_buf, (char *)grid1_buf, (char *)grid2_buf,
									  (int *)y0, (float *)y1, (float *)y2);

	env->ReleaseByteArrayElements(grid0_out, grid0_buf, JNI_ABORT);
	env->ReleaseByteArrayElements(grid1_out, grid1_buf, JNI_ABORT);
	env->ReleaseByteArrayElements(grid2_out, grid2_buf, JNI_ABORT);
	env->ReleaseIntArrayElements(ids, y0, 0);
	env->ReleaseFloatArrayElements(scores, y1, 0);
	env->ReleaseFloatArrayElements(boxes, y2, 0);

	return detect_counts;
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_rockchip_gpadc_demo_tracker_ObjectTracker_native_1create
                                                            (JNIEnv * env, jobject obj) {
    return create_tracker();
}

extern "C"
JNIEXPORT void JNICALL Java_com_rockchip_gpadc_demo_tracker_ObjectTracker_native_1destroy
                                                            (JNIEnv * env, jobject obj,
                                                            jlong handle) {
    destroy_tracker(handle);
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_rockchip_gpadc_demo_ImageBufferQueue_create_1npu_1img_1buffer
                                                            (JNIEnv * env, jobject obj, 
															jint img_format) {
    return create_npu_mem(img_format);
}

extern "C"
JNIEXPORT void JNICALL Java_com_rockchip_gpadc_demo_ImageBufferQueue_release_1npu_1img_1buffer
                                                            (JNIEnv * env, jobject obj,
                                                            jlong npu_buf_handle) {
   release_npu_mem(npu_buf_handle);
}


extern "C"
JNIEXPORT void JNICALL Java_com_rockchip_gpadc_demo_tracker_ObjectTracker_native_1track
                                                            (JNIEnv * env, jobject obj,
                                                                  jlong handle,
                                                                  jint maxTrackLifetime,
                                                                  jint track_input_num,
                                                                  jfloatArray track_input_locations,
                                                                  jintArray track_input_class,
                                                                  jfloatArray track_input_score,
                                                                  jintArray track_output_num,
                                                                  jfloatArray track_output_locations,
                                                                  jintArray track_output_class,
                                                                  jfloatArray track_output_score,
                                                                  jintArray track_output_id,
                                                                  jint width,
                                                                  jint height) {

	jboolean inputCopy = JNI_FALSE;
	jfloat *const c_track_input_locations = env->GetFloatArrayElements(track_input_locations,
																	   &inputCopy);
	jint *const c_track_input_class = env->GetIntArrayElements(track_input_class, &inputCopy);
	jfloat *const c_track_input_score = env->GetFloatArrayElements(track_input_score, &inputCopy);
	jboolean outputCopy = JNI_FALSE;

	jint *const c_track_output_num = env->GetIntArrayElements(track_output_num, &outputCopy);
	jfloat *const c_track_output_locations = env->GetFloatArrayElements(track_output_locations,
																		&outputCopy);
	jint *const c_track_output_class = env->GetIntArrayElements(track_output_class, &outputCopy);
	jfloat *const c_track_output_score = env->GetFloatArrayElements(track_output_score, &inputCopy);
	jint *const c_track_output_id = env->GetIntArrayElements(track_output_id, &outputCopy);


	track(handle, (int)maxTrackLifetime,
	        (int) track_input_num, (float *) c_track_input_locations, (int *) c_track_input_class, (float *)c_track_input_score,
		    (int *) c_track_output_num, (float *) c_track_output_locations, (int *) c_track_output_class, (float *)c_track_output_score,
		    (int *) c_track_output_id, (int) width, (int)height);

	env->ReleaseFloatArrayElements(track_input_locations, c_track_input_locations, JNI_ABORT);
	env->ReleaseIntArrayElements(track_input_class, c_track_input_class, JNI_ABORT);
    env->ReleaseFloatArrayElements(track_input_score, c_track_input_score, JNI_ABORT);

	env->ReleaseIntArrayElements(track_output_num, c_track_output_num, 0);
	env->ReleaseFloatArrayElements(track_output_locations, c_track_output_locations, 0);
	env->ReleaseIntArrayElements(track_output_class, c_track_output_class, 0);
    env->ReleaseFloatArrayElements(track_output_score, c_track_output_score, 0);
	env->ReleaseIntArrayElements(track_output_id, c_track_output_id, 0);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_rockchip_gpadc_demo_rga_RGA_color_1convert_1and_1flip(JNIEnv *env, jclass clazz,
                                                               jbyteArray src, jint src_fmt,
															   jlong npu_buf_handle,
                                                               jint dst_fmt,
                                                               jint width, jint height, jint flip) {
	jboolean copy = JNI_FALSE;
	jbyte* src_buf = env->GetByteArrayElements(src, &copy);
	//jbyte* dst_buf = env->GetByteArrayElements(dst, &copy);

	jint ret = colorConvertAndFlip(src_buf, src_fmt, npu_buf_handle, dst_fmt, width, height, flip);

	env->ReleaseByteArrayElements(src, src_buf, 0);
	// env->ReleaseByteArrayElements(dst, dst_buf, 0);

	return ret;
}