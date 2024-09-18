下述<TARGET_PLATFORM>是RV1106, RV1106B RV1103 或者 RV1103B

# Arm Linux Demo

## 编译

RV1106/RV1103编译脚本均为 `build-linux.sh`，设置交叉编译器所在目录的路径 `RK_RV1106_TOOLCHAIN`，例如修改成

```sh
export RK_RV1106_TOOLCHAIN=~/opts/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf
```

然后执行：

```
./build-linux.sh -t <TARGET_PLATFORM> -b Release
```

## 安装

连接设备并将构建输出推送到“/userdata”

```
adb push install/rknn_mobilenet_demo_Linux /userdata/
```

## 运行

```
adb shell
cd /userdata/rknn_mobilenet_demo_Linux/
```

```
export LD_LIBRARY_PATH=./lib
./rknn_mobilenet_demo model/<TARGET_PLATFORM>/mobilenet_v1.rknn model/dog_224x224.jpg
```

注意：RV1103B和RV1106B暂不支持NHWC输出，因此main_nhwc.c仅支持RV1103和RV1106。
