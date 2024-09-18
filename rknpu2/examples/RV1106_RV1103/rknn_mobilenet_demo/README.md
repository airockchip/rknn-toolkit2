The following <TARGET_PLATFORM> is RV1106, RV1106B RV1103 or RV1103B


# Arm Linux Demo

## build

The 'build-linux.sh' can be used for compiling demo for target including RV1106 and RV1103.

Changing the cross compiler path via the setting the `RK_RV1106_TOOLCHAIN`, shown as below:

```sh
export RK_RV1106_TOOLCHAIN=~/opts/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf
```

then, run the script：
```
./build-linux.sh -t <TARGET_PLATFORM> -b Release
```

## install

connect device and push build output into `/userdata`

```
adb push install/rknn_mobilenet_demo_Linux /userdata/
```

## run

```
adb shell
cd /userdata/rknn_mobilenet_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_mobilenet_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
```

Note: RV1103B and RV1106B do not support NHWC output yet, so main_nhwc.c only supports RV1103 and RV1106.