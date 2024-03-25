# RKNN Custom Pytorch OP Demo
This demo is an example of showing how to create, convert and run model with pytorch custom op.

## Model Source
The pytorch custom op model can be created in pytorch model and export to ONNX by following scripts under the folder 

**convert_rknn_demo**. The specific steps are shown below:

```shell
cd /path_to_rknn_custom_pytorch_op_demo/convert_rknn_demo/
```

then, export a pytorch custom op model with single custom op only to ONNX, 

Note: The details can be reffered to the example of pytorch custom op from rknn-toolkit2.

```shell
python3 generate_pytorch_custom_op_model.py 
```

Finally, convert to rknn model,

Note: Remeber to change the platform according to yours. 

```shell
python3 test.py
```

At this step, you should have a .rknn model on the current path. 

# Aarch64 Linux Demo

## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```shell
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## *Install*

Copy install/rknn_custom_cpu_op_demo_Linux  to the devices under /userdata/.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and RKNN model to `/userdata`

```shell
adb push install/rknn_custom_pytorch_op_demo_Linux/ /data/
```

- If board has sshd service, users can use scp or other methods to copy the program and RKNN model to the board.

## Run

```shell
adb shell
cd /data/rknn_custom_pytorch_op_demo_Linux
```

```shell
export LD_LIBRARY_PATH=./lib
./rknn_custom_pytorch_op_demo model/<TARGET_PLATFORM>/dual_residual_custom.rknn
model/dual_residual_input_0_nhwc.npy#model/dual_residual_input_1_nhwc.npy  1
```

# Android Demo

## Build

First export `ANDROID_NDK_PATH`, for example `export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`, then execute:

```shell
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# sush as: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

## Install

connect device and push build output into `/data`

```shell
 adb push install/rknn_custom_pytorch_op_demo_Android/ /data/
```

## Run

```shell
adb shell
cd /data/rknn_custom_pytorch_op_demo_Android/
```

then exporting LD_LIBRARY_PATH to the local path of lib.

```shell
export LD_LIBRARY_PATH=./lib
./rknn_custom_pytorch_op_demo model/<TARGET_PLATFORM>/dual_residual_custom.rknn
model/dual_residual_input_0_nhwc.npy#model/dual_residual_input_1_nhwc.npy  1
```

Note_1: Different platforms, different versions of tools and drivers may have slightly different results.

Note_2: This demo model has two inputs and outputs and they have been provided under folder **model** for better verifying. Users can have their own inputs for their own models.

Note_3: Becareful for input layout for your input own data that should be matched from rknn_query .

Note_4: The input npy for current rknn_custom_cpu_op_demo only supports NHWC or Undefined layout !

