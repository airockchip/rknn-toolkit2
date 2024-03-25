# RKNN C API 动态形状输入Demo
这是一个使用RKNN C API进行动态形状输入推理的演示应用。您可以在这个应用中看到如何使用RKNN 动态形状 C API对图像进行分类。

# 如何使用
1. 克隆或下载此代码库https://github.com/rockchip-linux/rknn-toolkit2/tree/master/rknpu2。
2. 在终端中进入动态形状推理Demo目录。
```
cd examples/rknn_dynamic_shape_input_demo
```
3. 根据芯片平台，运行shell脚本编译应用程序,以RK3562 Android系统为例，命令如下:
```
./build-android_RK3562.sh
```
4. 将Demo程序目录通过adb命令推送到开发板系统中，命令如下:
```
adb push ./install/rknn_dynshape_demo_Android/ /data
注意：如果是安卓系统，需要adb root & adb remount
```
5. 设置runtime库链接路径
```
export LD_LIBRARY_PATH=./lib
```
6. 运行程序，以rk3562平台为例，./rknn_dynshape_inference model/RK3562/mobilenet_v2.rknn images/dog_224x224.jpg 命令对图像进行分类，其中 mobilenet_v2.rknn 是神经网络模型文件的名称，dog_224x224.jpg 是要分类的图像文件的名称。


# 编译说明
## Arm Linux系统

首先导入GCC_COMPILER，例如`export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu 
`，然后执行如下命令：

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# 例如: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```
## Android系统

首先导入ANDROID_NDK_PATH，例如`export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`，然后执行如下命令：

```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# 例如: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

# 包含的功能
此演示应用程序包含以下功能：

- 创建一个包含动态形状的神经网络模型。
参考https://github.com/rockchip-linux/rknn-toolkit2仓库下的examples/functions/dynamic_input

- 从文件中读取一张图像，并使用神经网络模型对其进行分类。程序步骤如下：
1. 使用 rknn_init() 函数初始化 RKNN 上下文。
2. 使用 rknn_set_input_shapes() 函数设置模型所有的输入的形状信息，包括形状、布局等。
3. 使用 rknn_query() 函数查询当前设置的模型输入和输出的信息，包括形状、数据类型和大小等。
4. 使用 rknn_inputs_set() 函数设置模型输入的数据，包括数据指针和数据大小等。
5. 使用 rknn_run() 函数运行模型。
6. 使用 rknn_outputs_get() 函数设置是否需要float类型结果并获取输出数据。
7. 处理输出数据，得到分类结果和概率。
8. 使用 rknn_release() 函数释放RKNN上下文。