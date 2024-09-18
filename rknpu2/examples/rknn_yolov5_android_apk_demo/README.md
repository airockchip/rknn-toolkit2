# Introduction
 - rknn_yolov5_android_apk_demo is the one showing how to deploy yolov5s model on the android apk on RK3566_RK3568, RK3562 or RK3588



# Pre-requisite

 - Using the andorid studio 2021 1.2.1 or 1.3.1, higher version
 - **RGA requires driver version higher than 1.2.4 and librga.so higher than 1.4** 

 - Yolov5s.rknn is required, which can be converted from yolov5s.onnx, which SiLU activation layer was replace with ReLU. The detail should refer to this link: https://github.com/airockchip/rknn_model_zoo/blob/main/examples/yolov5/README.md 


# Structure

## The demo can be split into two parts:
 - JAVA: com.rockchip.gpadc.demo: Reading camera inputs, and invoking jni interface to inferece and show the result

 - JNI: The interface for invoking rknnrt to do the inference of model.



# Permission
  This demo requires the permission of Camera and Read/Write permission for EXTERNAL_STORAGE


# FAQ

## Compiling error like“files found with path 'lib/arm64-v8a/xxx.so' from inputs:”

```
Execution failed for task ':app:mergeDebugNativeLibs'.
> A failure occurred while executing com.android.build.gradle.internal.tasks.MergeNativeLibsTask$MergeNativeLibsTaskWorkAction
   > 2 files found with path 'lib/arm64-v8a/librga.so' from inputs:
      - rknpu2/examples/rknn_yolov5_android_apk_demo/app/build/intermediates/merged_jni_libs/debug/out/arm64-v8a/librga.so
      - rknpu2/examples/rknn_yolov5_android_apk_demo/app/build/intermediates/cxx/Debug/3f40b265/obj/arm64-v8a/librga.so
     If you are using jniLibs and CMake IMPORTED targets, see
     https://developer.android.com/r/tools/jniLibs-vs-imported-targets

* Try:
> Run with --info or --debug option to get more log output.
> Run with --scan to get full insights.
```
Alternatively,
```
2 files found with path 'lib/arm64-v8a/xxx.so' from inputs:
 - rknn_yolov5_android_apk_demo/app/build/intermediates/merged_jni_libs/debug/out/arm64-v8a/librga.so
 - rknn_yolov5_android_apk_demo/app/build/intermediates/cxx/Debug/3f40b265/obj/arm64-v8a/librga.so
If you are using jniLibs and CMake IMPORTED targets, see
https://developer.android.com/r/tools/jniLibs-vs-imported-targets
```
This requires add the "jniLibs.srcDirs = ['libs']" on app/build.gradle file “https://developer.android.com/r/tools/jniLibs-vs-imported-targets”



## During launch app，errors like "E/SurfaceView: Exception configuring surface"

```
D/rkyolo: camera facing: 1
V/rkyolo: Camera Supported Preview Size = 160x90
V/rkyolo: Camera Supported Preview Size = 320x180
V/rkyolo: Camera Supported Preview Size = 432x240
V/rkyolo: Camera Supported Preview Size = 640x360
V/rkyolo: Camera Supported Preview Size = 800x448
V/rkyolo: Camera Supported Preview Size = 864x480
V/rkyolo: Camera Supported Preview Size = 1024x576
V/rkyolo: Camera Supported Preview Size = 1280x720
V/rkyolo: Camera Supported Preview Size = 1600x896
V/rkyolo: Camera Supported Preview Size = 1920x1080
E/SurfaceView: Exception configuring surface
    java.lang.RuntimeException: setParameters failed
        at android.hardware.Camera.native_setParameters(Native Method)
        at android.hardware.Camera.setParameters(Camera.java:2068)
        at com.rockchip.gpadc.demo.MainActivity.setCameraParameters(MainActivity.java:295)
        at com.rockchip.gpadc.demo.MainActivity.startCamera(MainActivity.java:245)
        at com.rockchip.gpadc.demo.MainActivity.access$300(MainActivity.java:47)
        at com.rockchip.gpadc.demo.MainActivity$TSurfaceHolderCallback.surfaceCreated(MainActivity.java:199)
```

This is casued by the unsupported camera resolution , modifying CAMERA_PREVIEW_WIDTH in "app/src/main/java/com/rockchip/gpadc/demo/rga/HALDefine.java" for supported resolution.