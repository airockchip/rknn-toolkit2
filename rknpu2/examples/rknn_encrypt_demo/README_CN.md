以下 <TARGET_PLATFORM> 表示RK3566_RK3568、RK3562、RK3576、RK3588、RV1126B、RV1103、RV1106、RV1103B。


# 概述

该示例用于说明**加密**的RKNN模型并在NPU上进行推理。使用RKNPU运行时提供的
混合加密方案（AES-256-CTR + RSA-OAEP）。

密钥信封通过`rknn_init_extend.user_key`传入`rknn_init()`，运行时在`rknn_init()`内部
透明地完成解密。解密完成后，后续推理流程与明文模型完全一致。

# 第一步：加密模型（在PC/服务器端）

使用`convert/`目录下的加密脚本对明文`.rknn`模型进行加密。需要准备RK公钥
（`rk_public_key.pem`，由Rockchip提供）并安装`pycryptodome`（`pip install pycryptodome`）。

```
cd convert
python3 rknn_model_encrypt.py --model ../model/<TARGET_PLATFORM>/mobilenetv2-12.rknn \
                              --rk-pubkey rk_public_key.pem \
                              --output-dir ../model/<TARGET_PLATFORM>/
```

执行后会生成两个文件：

| 文件 | 说明 |
|------|------|
| `mobilenetv2-12.rknn` | 加密后的模型（覆盖输出目录中的明文模型） |
| `mobilenetv2-12_key.bin` | RSA-OAEP加密的密钥信封（与模型一起部署到板端） |

> **注意**：请妥善保管明文模型。部署到板端的是加密后的`mobilenetv2-12.rknn`和
> `mobilenetv2-12_key.bin`。

# 第二步：编译和运行Demo

# Aarch64 Linux 示例
## 编译

首先导入GCC_COMPILER，例如`export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu
`，然后执行如下命令：

```
./build-linux.sh -t <target> -a <arch> [-b <build_type>]

# 例如:
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## 安装

将 install/rknn_encrypt_demo_Linux 拷贝到设备上。

- 如果使用Rockchip的EVB板，可以使用以下命令：

连接设备并将程序和模型传输到`/userdata`

```
adb push install/rknn_encrypt_demo_Linux /userdata/
```

- 如果你的板子有sshd服务，可以使用scp命令或者其他方式将程序和模型传输到板子上。

## 运行


```
adb shell
cd /userdata/rknn_encrypt_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_encrypt_demo model/<TARGET_PLATFORM>/mobilenetv2-12.rknn model/<TARGET_PLATFORM>/mobilenetv2-12_key.bin model/dog_224x224.jpg
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
adb push install/rknn_encrypt_demo_Android /data/
```

## 运行

```
adb shell
cd /data/rknn_encrypt_demo_Android/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_encrypt_demo model/<TARGET_PLATFORM>/mobilenetv2-12.rknn model/<TARGET_PLATFORM>/mobilenetv2-12_key.bin model/dog_224x224.jpg
```

# 用法说明

```
./rknn_encrypt_demo <enc_model.rknn> <key_envelope.bin> [input_path] [loop_count] [core_mask]
```

| 参数 | 说明 |
|------|------|
| enc_model.rknn | 加密后的RKNN模型文件路径 |
| key_envelope.bin | RSA-OAEP密钥信封文件路径 |
| input_path | （可选）输入图片路径。如果不提供，则输入全零数据 |
| loop_count | （可选）推理循环次数，默认为1 |
| core_mask | （可选）NPU核心掩码（0=自动, 1=core0, 2=core1, ...），默认为1。仅在多核平台生效 |

# 注意
 - 支持平台：RK3562/RK3566/RK3568/RK3576/RK3588/RV1126B/RV1106/RV1103B
 - RK公钥（`rk_public_key.pem`）由Rockchip提供
 - 关于加密方案的更多细节，请参考RKNN User Guide中的"模型加密"章节
