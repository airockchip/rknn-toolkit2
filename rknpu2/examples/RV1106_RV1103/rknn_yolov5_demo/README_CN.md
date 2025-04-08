# Yolo-v5 demo
下述<TARGET_PLATFORM>是RV1106, RV1106B RV1103 或者 RV1103B

# 导出rknn模型

请参考 https://github.com/airockchip/rknn_model_zoo/tree/main/models/CV/object_detection/yolo



## Arm Linux Demo

### 编译

RV1106/RV1103编译脚本均为 `build-linux.sh`，设置交叉编译器所在目录的路径 `RK_RV1106_TOOLCHAIN`，例如修改成

```sh
export RK_RV1106_TOOLCHAIN=~/opts/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf
```

然后执行：

```sh
./build-linux.sh -t <TARGET_PLATFORM> -b Release
```

### 推送执行文件到板子

连接板子的usb口到PC,将整个demo目录到 `/userdata`:

```sh
adb push install/rknn_yolov5_demo_Linux /userdata/
```

### 运行

```sh
adb shell
cd /userdata/rknn_yolov5_demo_Linux/

export LD_LIBRARY_PATH=/userdata/rknn_yolov5_demo_Linux/lib
./rknn_yolov5_demo model/RV1106/yolov5s-640-640.rknn model/bus.jpg
```

Note:

- LD_LIBRARY_PATH 必须采用全路径
- 基于性能原因，demo中将 RKNN 模型的输出 fmt 设置为 RKNN_QUERY_NATIVE_NHWC_OUTPUT_ATTR，以获取更好的推理性能。此时模型输出 buf 是以 NHWC 顺序进行排布的，比如第一个输出的原始 shape 是 1,255,80,80，此时RKNN输出的 shape 是1,80,80,255，此demo中的后处理也根据这个顺序做了相应的优化调整。
- RV1103B和RV1106B暂不支持NHWC输出,为了获取更好的性能我们直接支持了NC1HWC2排布的后处理
