# 简介
 - rknn_yolov5_android_apk_demo 是RK3566_RK3568, RK3562或RK3588上如何调用NPU的demo，该demo的基础模型是yolov5s



# 使用说明

 - 可以使用android studio 2021.2.1 （Windows）或者android studio 2021.3.1（Linux/macOS） 编译该工程
 - **RGA版本要求： 驱动版本大于1.2.4，librga 版本大于1.4**
 - yolov5s.rknn是使用rknn toolkit2将yolov5s.onnx转换而来，其中激活函数用relu替换silu，具体转换方法参考: https://github.com/airockchip/rknn_model_zoo/blob/main/examples/yolov5/README.md 

# 代码说明

## 代码分为两大部分：
 - JAVA: com.rockchip.gpadc.demo: 读取camera输入，并调用jni进行inference，并将结果显示出来

 - JNI: 调用rknnrt进行实际inference



# 权限
   运行本程序需要Camera及EXTERNAL_STORAGE读写权限


# FAQ

## 编译时出现“files found with path 'lib/arm64-v8a/xxx.so' from inputs:”类似错误
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
或者
```
2 files found with path 'lib/arm64-v8a/xxx.so' from inputs:
 - rknn_yolov5_android_apk_demo/app/build/intermediates/merged_jni_libs/debug/out/arm64-v8a/librga.so
 - rknn_yolov5_android_apk_demo/app/build/intermediates/cxx/Debug/3f40b265/obj/arm64-v8a/librga.so
If you are using jniLibs and CMake IMPORTED targets, see
https://developer.android.com/r/tools/jniLibs-vs-imported-targets
```
则需要添加 app/build.gradle中的"jniLibs.srcDirs = ['libs']"，具体原因参考“https://developer.android.com/r/tools/jniLibs-vs-imported-targets”



## app启动时，出现"E/SurfaceView: Exception configuring surface"的错误

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

是因为app设置的分辨率该camera不支持，需要修改"app/src/main/java/com/rockchip/gpadc/demo/rga/HALDefine.java"中CAMERA_PREVIEW_WIDTH及CAMERA_PREVIEW_HEIGHT为摄像头支持的分辨率。