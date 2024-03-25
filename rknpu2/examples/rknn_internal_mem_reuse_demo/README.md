# rknn_internal_mem_reuse_demo

## Description

This project is mainly used to demonstrate the usage of **RKNN_FLAG_MEM_ALLOC_OUTSIDE** and **rknn_set_internal_mem**.

RKNN_FLAG_MEM_ALLOC_OUTSIDE has two main purposes:

- All memory is allocated by the user, allowing for better overall memory management.
- It enables memory reuse, especially for memory-constrained chips like RV1103/RV1106.

Assuming there are two models, Model A and Model B, designed to run sequentially, the intermediate tensor memory between these two models can be reused. The example code is as follows:

```
rknn_context ctx_a, ctx_b;

rknn_init(&ctx_a, model_path_a, 0, RKNN_FLAG_MEM_ALLOC_OUTSIDE, NULL);
rknn_query(ctx_a, RKNN_QUERY_MEM_SIZE, &mem_size_a, sizeof(mem_size_a));

rknn_init(&ctx_b, model_path_b, 0, RKNN_FLAG_MEM_ALLOC_OUTSIDE, NULL);
rknn_query(ctx_b, RKNN_QUERY_MEM_SIZE, &mem_size_b, sizeof(mem_size_b));

max_internal_size = MAX(mem_size_a.total_internal_size, mem_size_b.total_internal_size);
internal_mem_max = rknn_create_mem(ctx_a, max_internal_size);

internal_mem_a = rknn_create_mem_from_fd(ctx_a, internal_mem_max->fd,
        internal_mem_max->virt_addr, mem_size_a.total_internal_size, 0);
rknn_set_internal_mem(ctx_a, internal_mem_a);

internal_mem_b = rknn_create_mem_from_fd(ctx_b, internal_mem_max->fd,
        internal_mem_max->virt_addr, mem_size_a.total_internal_size, 0);
rknn_set_internal_mem(ctx_b, internal_mem_b);
```

Note: This demo uses the RKNN models from the rknn_mobilenet_demo and rknn_yolov5_demo example	 in the parent directory. Make sure they exist before compiling.

## Android Demo

### Compilation

First export `ANDROID_NDK_PATH`, for example `export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`, then execute:

```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# sush as: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

### Pushing the Executable to the Device

Connect the device to the PC via USB and push the entire demo directory to `/data`:

```
adb remount
adb push install/rknn_internal_mem_reuse_demo_Android /data/
```

### Execution

```
cd /data/rknn_internal_mem_reuse_demo_Android/

export LD_LIBRARY_PATH=./lib
./rknn_internal_mem_reuse_demo model/<TARGET_PLATFORM>/yolov5s-640-640.rknn model/bus.jpg model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/cat_224x224.jpg
```

## Aarch64 Linux Demo

### Compilation

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

### Pushing the Executable to the Device

Copy the `install/rknn_internal_mem_reuse_demo_Linux` to the `/userdata/` directory on the target device.

- If using a Rockchip EVB board, use adb to push the file to the device:

```
bashCopy code
adb push install/rknn_internal_mem_reuse_demo_Linux /userdata/
```

- If using other board, use scp or other methods to copy `install/rknn_internal_mem_reuse_demo_Linux` to the `/userdata/` directory on the device.

### Execution

```
adb shell
cd /userdata/rknn_internal_mem_reuse_demo_Linux/

export LD_LIBRARY_PATH=./lib
./rknn_internal_mem_reuse_demo model/<TARGET_PLATFORM>/yolov5s-640-640.rknn model/bus.jpg model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/cat_224x224.jpg
```
