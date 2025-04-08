The following <TARGET_PLATFORM> represents RK3566_RK3568, RK3562, RK3576, RK3588, RV1126B.
# Aarch64 Linux Demo
## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## Install

Copy install/rknn_api_demo_Linux to the devices.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and rknn model to `/userdata`

```
adb push install/rknn_api_demo_Linux /userdata/
```

- If your board has sshd service, you can use scp or other methods to copy the program and rknn model to the board.

## Run

```
adb shell
cd /userdata/rknn_api_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_create_mem_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
./rknn_create_mem_with_rga_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
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
adb push install/rknn_api_demo_Android /data/
```

## Run

```
adb shell
cd /data/rknn_api_demo_Android/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_create_mem_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
./rknn_create_mem_with_rga_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
./rknn_with_mmz_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
./rknn_set_internal_mem_from_fd_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
```

# Note
 - You may need to update libmpimmz.so and its header file of this project according to the implementation of MMZ in the system.
 - You may need to update librga.so and its header file of this project according to the implementation of RGA in the system. https://github.com/airockchip/librga.
    For rk3562, the librga version need to be 1.9.1 or higher.
 - You may need to use r19c or older version of ndk for compiling with MMZ related demo.
 - The test model comes from https://github.com/rockchip-linux/rknn-toolkit2/tree/master/rknn-toolkit2/examples/tflite/mobilenet_v1.