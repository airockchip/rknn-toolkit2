The following <TARGET_PLATFORM> represents RK3566_RK3568,RK3562,RK3576,RK3588,RV1126B.
# Aarch64 Linux Demo
## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## Install

Copy install/rknn_multiple_input_demo_Linux to the devices under `/userdata`.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and rknn model to `/userdata`

```
adb push install/rknn_multiple_input_demo_Linux /userdata/
```

- If your board has sshd service, you can use scp or other methods to copy the program and rknn model to the board.

## Run

```
adb shell
cd /userdata/rknn_multiple_input_demo_Linux/
```



```
export LD_LIBRARY_PATH=./lib
./rknn_multiple_input_demo model/<TARGET_PLATFORM>/multiple_input_demo.rknn model/input1.bin#model/input2.bin
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
adb push install/rknn_multiple_input_demo_Android /data/
```

## Run

```
adb shell
cd /data/rknn_multiple_input_demo_Android/
```



```
export LD_LIBRARY_PATH=./lib
./rknn_multiple_input_demo model/<TARGET_PLATFORM>/multiple_input_demo.rknn model/input1.bin#model/input2.bin
```