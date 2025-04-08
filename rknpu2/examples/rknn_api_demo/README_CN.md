以下 <TARGET_PLATFORM> 表示RK3566_RK3568、RK3562、RK3576、RK3588、RV1126B。
# Aarch64 Linux 示例
## 编译
首先导入GCC_COMPILER，例如`export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu 
`，然后执行如下命令：

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# 例如: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## 安装

将 install/rknn_api_demo_Linux 拷贝到设备上。

- 如果使用Rockchip的EVB板，可以使用以下命令：

连接设备并将程序和模型传输到`/userdata`

```
adb push install/rknn_api_demo_Linux /userdata/
```

- 如果你的板子有sshd服务，可以使用scp命令或者其他方式将程序和模型传输到板子上。

## 运行

```
adb shell
cd /userdata/rknn_api_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_create_mem_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
./rknn_create_mem_with_rga_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
```

# Android 示例
## 编译
首先导入ANDROID_NDK_PATH，例如`export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`，然后执行如下命令：
```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# 例如: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

## 安装

连接设备并将程序和模型传输到`/data`

```
adb push install/rknn_api_demo_Android /data/
```

## 运行

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

# 注意：
- 你可能需要依赖系统中的MMZ实现更新libmpimmz.so和头文件
- 你可能需要依赖系统中的RGA实现更新librga.so和头文件。库地址：https://github.com/airockchip/librga。对于RK3562,librga库版本需要大于等于1.9.1
- 你可能需要使用r19c或者更老的NDK版本编译MMZ相关的Demo
- 测试模型来源于：https://github.com/rockchip-linux/rknn-toolkit2/tree/master/rknn-toolkit2/examples/tflite/mobilenet_v1
