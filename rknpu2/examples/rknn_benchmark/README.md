rknn_benchmark is used to test the performance of the rknn model. Please make sure that the cpu/ddr/npu has been clocked to the highest frequency before testing.

Usage:

./rknn_benchmark xxx.rknn [input_data]  [loop_count] [core_mask]

core_mask: 0: auto, 1: npu core1, 2: npu core2, 4:npu core3,

​                     3: npu core1&2,

​                     7: npu core1&2&3

Only RK3588 support core mask.

Such as:

```
./rknn_benchmark mobilenet_v1.rknn
./rknn_benchmark mobilenet_v1.rknn dog.jpg 10 3
./rknn_benchmark mobilenet_v1.rknn dog.npy 10 7
./rknn_benchmark xxx.rknn input1.npy#input2.npy
```


The following <TARGET_PLATFORM> represents RK3566_RK3568, RK3562 or RK3588

# Aarch64 Linux Demo
## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## Install

Copy install/rknn_benchmark_Linux to the devices under /userdata/.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and rknn model to `/userdata`

```
adb push install/rknn_benchmark_Linux /userdata/
```

- If your board has sshd service, you can use scp or other methods to copy the program and rknn model to the board.

## Run

```
adb shell
cd /userdata/rknn_benchmark_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_benchmark xxx.rknn
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
adb push install/rknn_benchmark_Android /data/
```

## Run

```
adb shell
cd /data/rknn_benchmark_Android/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_benchmark xxx.rknn
```
