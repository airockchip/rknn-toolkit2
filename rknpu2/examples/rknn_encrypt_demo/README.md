The following <TARGET_PLATFORM> represents RK3566_RK3568, RK3562, RK3576, RK3588, RV1126B, RV1103, RV1106, RV1103B.


# Overview

This demo shows how to load an **encrypted** RKNN model and run inference on the
NPU. It uses the hybrid encryption scheme (AES-256-CTR + RSA-OAEP) provided by
the RKNPU runtime.

The key envelope is passed to `rknn_init()` via `rknn_init_extend.user_key`, and
the runtime decrypts the model transparently inside `rknn_init()`. After that,
the inference flow is identical to a plaintext model.

# Step 1: Encrypt the model (on PC/Server)

Use the encryption script in the `convert/` directory to encrypt a plaintext
`.rknn` model. You need the RK public key (`rk_public_key.pem`, provided by
Rockchip) and `pycryptodome` installed (`pip install pycryptodome`).

```
cd convert
python3 rknn_model_encrypt.py --model ../model/<TARGET_PLATFORM>/mobilenetv2-12.rknn \
                              --rk-pubkey rk_public_key.pem \
                              --output-dir ../model/<TARGET_PLATFORM>/
```

This produces two files:

| File | Description |
|------|-------------|
| `mobilenetv2-12.rknn` | The encrypted model (overwrites the plaintext model in the output dir) |
| `mobilenetv2-12_key.bin` | The RSA-OAEP encrypted key envelope (deploy alongside the model) |

> **Note**: Keep the plaintext model safe. The encrypted `mobilenetv2-12.rknn`
> and `mobilenetv2-12_key.bin` are what you deploy to the board.

# Step 2: Build and run the demo

# Aarch64 Linux Demo

## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> [-b <build_type>]

# such as:
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## Install

Copy install/rknn_encrypt_demo_Linux to the devices.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program and rknn model to `/userdata`

```
adb push install/rknn_encrypt_demo_Linux /userdata/
```

- If your board has sshd service, you can use scp or other methods to copy the program and rknn model to the board.

## Run

```
adb shell
cd /userdata/rknn_encrypt_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_encrypt_demo model/<TARGET_PLATFORM>/mobilenetv2-12.rknn model/<TARGET_PLATFORM>/mobilenetv2-12_key.bin model/dog_224x224.jpg
```

# Android Demo

## Build

First export `ANDROID_NDK_PATH`, for example `export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`, then execute:

```
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# such as:
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

## Install

connect device and push build output into `/data`

```
adb push install/rknn_encrypt_demo_Android /data/
```

## Run

```
adb shell
cd /data/rknn_encrypt_demo_Android/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_encrypt_demo model/<TARGET_PLATFORM>/mobilenetv2-12.rknn model/<TARGET_PLATFORM>/mobilenetv2-12_key.bin model/dog_224x224.jpg
```

# Usage

```
./rknn_encrypt_demo <enc_model.rknn> <key_envelope.bin> [input_path] [loop_count] [core_mask]
```

| Parameter | Description |
|-----------|-------------|
| enc_model.rknn | Path to the encrypted RKNN model file |
| key_envelope.bin | Path to the RSA-OAEP key envelope file |
| input_path | (Optional) Path to the input image. If omitted, zeros are fed as input |
| loop_count | (Optional) Number of inference loops, default 1 |
| core_mask | (Optional) NPU core mask (0=auto, 1=core0, 2=core1, ...), default 1. Only effective on multi-core platforms |

# Note
 - Supported platforms: RK3562/RK3566/RK3568/RK3576/RK3588/RV1126B/RV1106/RV1103B
 - The RK public key (`rk_public_key.pem`) is provided by Rockchip. 
 - For more details about the encryption scheme, please refer to the "Model Encryption" chapter in the RKNN User Guide.
