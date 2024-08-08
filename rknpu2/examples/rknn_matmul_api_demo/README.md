rknn_matmul_api_demo is a example which performs matrix multiplication using the RKNPU matmul C API.

Usage:

1. static shape mode

```
Usage:
./rknn_matmul_api_demo <matmul_type> <M,K,N> <B_layout> <AC_layout> <loop_count> <core_mask> <print_result> <iommu_domain_id>
     matmul_type = 1: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32
     matmul_type = 2: RKNN_INT8_MM_INT8_TO_INT32
     matmul_type = 4: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16
     matmul_type = 7: RKNN_FLOAT16_MM_INT4_TO_FLOAT32
     matmul_type = 10: RKNN_INT4_MM_INT4_TO_INT16
Example: A = [4,64], B = [64,32], int8 matmul test command as followed:
./rknn_matmul_api_demo 2 4,64,32
```



2. dynamic shape mode

   ```sh
   ./rknn_matmul_api_dynshape_demo <matmul_type> <M1K1N1#M2K2N2#...> <B_layout> <AC_layout> <loop_count> <core_mask>
        M_shapes:         M shape array, which separeted by ',' 
        matmul_type = 1: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32
        matmul_type = 2: RKNN_INT8_MM_INT8_TO_INT32
        matmul_type = 4: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16
        matmul_type = 7: RKNN_FLOAT16_MM_INT4_TO_FLOAT32
        matmul_type = 10: RKNN_INT4_MM_INT4_TO_INT16
   Example: A = [1,64]#[4,64]#[8,64], B = [64,32], int8 matmul test command as followed:
     feature+const: ./rknn_matmul_api_dynshape_demo 2 1,64,32#4,64,32#8,64,32 1 1
     two feature:   ./rknn_matmul_api_dynshape_demo 2 1,64,32#4,64,32#8,64,32 2 1
   ```

   

# Aarch64 Linux Demo
## Build

First export `GCC_COMPILER`, for example `export GCC_COMPILER=~/opt/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu`, then execute:

```
./build-linux.sh -t <target> -a <arch> -b <build_type>]

# such as: 
./build-linux.sh -t rk3588 -a aarch64 -b Release
```

## Install

Copy install/rknn_matmul_api_demo_Linux to the devices under /userdata/.

- If you use rockchip's evb board, you can use the following way:

Connect device and push the program to `/userdata`

```
adb push install/rknn_matmul_api_demo_Linux /userdata/
```

- If your board has sshd service, you can use scp or other methods to copy the program to the board.

## Run

```
adb shell
cd /userdata/rknn_matmul_api_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_matmul_api_demo 2 4 64 32
```

or

```sh
export LD_LIBRARY_PATH=./lib
./rknn_matmul_api_dynshape_demo 2 1,64,32#4,64,32#8,64,32 1 1
```



# Android Demo

## Build

First export `ANDROID_NDK_PATH`, for example `export ANDROID_NDK_PATH=~/opts/ndk/android-ndk-r18b`, then execute:

```sh
./build-android.sh -t <target> -a <arch> [-b <build_type>]

# sush as: 
./build-android.sh -t rk3568 -a arm64-v8a -b Release
```

## Install

connect device and push build output into `/data`

```sh
adb push install/rknn_matmul_api_demo_Android /data/
```

## Run

```
adb shell
cd /data/rknn_matmul_api_demo_Android/
```

```sh
export LD_LIBRARY_PATH=./lib
./rknn_matmul_api_demo 2 4 64 32
```

or

```sh
export LD_LIBRARY_PATH=./lib
./rknn_matmul_api_dynshape_demo 2 1,64,32#4,64,32#8,64,32 1 1
```

