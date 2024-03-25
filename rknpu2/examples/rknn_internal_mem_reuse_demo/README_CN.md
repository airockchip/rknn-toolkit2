# rknn_internal_mem_reuse_demo

## 说明

本工程主要用于**RKNN_FLAG_MEM_ALLOC_OUTSIDE** 及 **rknn_set_internal_mem**的使用演示。

RKNN_FLAG_MEM_ALLOC_OUTSIDE：主要有两方面的作用:
- 所有内存均是用户自行分配，便于对整个系统内存进行统筹安排
- 用于内存复用，特别是针对RV1103/RV1106这种内存极为紧张的情况。


假设有模型A、B 两个模型，这两个模型在设计上串行运行的，那么这两个模型的中间tensor的内存就可以复用。示例代码如下：
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



注意：本工程使用了上级目录的rknn_mobilenet_demo及rknn_yolov5_demo两个工程的rknn模型，编译之前请确保其存在。

## Android Demo

### 编译

首先导入ANDROID_NDK_PATH，例如`export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`，然后执行如下命令：

```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# 例如: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

### 推送执行文件到板子

连接板子的usb口到PC,将整个demo目录到 `/data`:

```sh
adb root
adb remount
adb push install/rknn_internal_mem_reuse_demo_Android /data/
```

### 运行

```sh
adb shell
cd /data/rknn_internal_mem_reuse_demo_Android/

export LD_LIBRARY_PATH=./lib
./rknn_internal_mem_reuse_demo model/<TARGET_PLATFORM>/yolov5s-640-640.rknn model/bus.jpg model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/cat_224x224.jpg
```

## Aarch64 Linux Demo

### 编译

首先导入GCC_COMPILER，例如`export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu 
`，然后执行如下命令：

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# 例如: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

### 推送执行文件到板子


将 install/rknn_internal_mem_reuse_demo_Linux 拷贝到板子的/userdata/目录.

- 如果使用rockchip的EVB板子，可以使用adb将文件推到板子上：

```
adb push install/rknn_internal_mem_reuse_demo_Linux /userdata/
```

- 如果使用其他板子，可以使用scp等方式将install/rknn_internal_mem_reuse_demo_Linux拷贝到板子的/userdata/目录

### 运行

```sh
adb shell
cd /userdata/rknn_internal_mem_reuse_demo_Linux/

export LD_LIBRARY_PATH=./lib
./rknn_internal_mem_reuse_demo model/<TARGET_PLATFORM>/yolov5s-640-640.rknn model/bus.jpg model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/cat_224x224.jpg
```

