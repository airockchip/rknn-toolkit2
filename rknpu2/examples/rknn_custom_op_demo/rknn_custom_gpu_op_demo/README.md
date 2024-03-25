# RKNN Custom GPU op Demo
This demo is an example of showing how to implement custom gpu op on c demo for rknn model, using 'ArgMax' as an example.

The example model is the pp-human-seg-v2_mobile from PaddleSeg. It is recommended referring to RKNN-Toolkit2 for  how to replace existed onnx op, referring to  [README.md](model/convert/README.md) for more details.

## Model Source

The example demo comes from open source project of PaddleSeg. The specfication can be seen with the following link: [PP-HumanSeg](https://github.com/PaddlePaddle/PaddleSeg/tree/release/2.9/contrib/PP-HumanSeg)  

The example model is the PP-HumanSegV2-Mobile, which can be downloaded with this link: [PP-HumanSegV2-Mobile](https://paddleseg.bj.bcebos.com/dygraph/pp_humanseg_v2/human_pp_humansegv1_mobile_192x192_inference_model.zip).

Note: Referring to Paddle2onnx for converting paddle model to onnx model.

The converted RKNN models of platforms RK3562, RK3566, RK3568,RK3576, RK3588 are located in model folder.

# Aarch64 Linux Demo

## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## *Install*

Copy install/rknn_custom_gpu_op_demo_Linux  to the devices under /userdata/.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and rknn model to `/userdata`

```
adb push install/rknn_custom_gpu_op_demo_Linux/ /data/
```

- If your board has sshd service, you can use scp or other methods to copy the program and rknn model to the board.

## Run

```
adb shell
cd /data/rknn_custom_gpu_op_demo_Linux
```

```shell
export LD_LIBRARY_PATH=./lib
./rknn_custom_gpu_op_demo model/<TARGET_PLATFORM>/pp_human_segv2_custom_argmax.rknn model/test_image.jpg 1
```

# Android Demo

## Build

First export `ANDROID_NDK_PATH`, for example `export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`, then execute:

```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# sush as: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

## Install

connect device and push build output into `/data`

```
 adb push install/rknn_custom_gpu_op_demo_Android/ /data/
```

## Run

```
adb shell
cd /data/rknn_custom_gpu_op_demo_Android/
```

If got this type of error library "libOpenCL.so" not found: needed by main executable, by copying the 'libOpenCL.so' to local path to solve it, for instance,

```shell
cp /vendor/lib64/libOpenCL.so ./lib/
```

then exporting LD_LIBRARY_PATH to the local path of lib.

```
export LD_LIBRARY_PATH=./lib
./rknn_custom_gpu_op_demo model/<TARGET_PLATFORM>/pp_human_segv2_custom_argmax.rknn model/test_image.jpg 1
```

Note: Different platforms, different versions of tools and drivers may have slightly different results.

## Plug-in Librarys

If users want to verify the accuracy of their own implemented custom op,  the accuracy analysis from the rknn-toolkit2 should be used for this purpose. To do this, the plug-in library need to be compiled for single custom op implmentation for accuracy analysis. 

Compile plug-in library, the source file is located at the **./src/rknn_custom_op_opencl_plugin_lib.cpp**

```sh
./build-linux_<TARGET_PLATFORM>.sh
or
./build-android_<TARGET_PLATFORM>.sh
```

then push plug-in library to the path for Linux **/usr/lib/rknpu/op_plugins**, for android, **/vendor/lib64/（Android arm64-v8a）**, for example, for Linux system,

```sh
adb push install/rknn_custom_gpu_op_demo_Linux/lib/librkcst_argmax.so /usr/lib/rknpu/op_plugins
```

Note: if there is no such directory on your board, users should create one instead.

At last, users can run the accuracy analysis from toolkit2 for veirying whether it is accurate for their custom op.
