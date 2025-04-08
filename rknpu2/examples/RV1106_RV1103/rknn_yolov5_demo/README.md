# Yolo-v5 demo
The following <TARGET_PLATFORM> is RV1106, RV1106B RV1103 or RV1103B.

# Export RKNN Model

Please refer https://github.com/airockchip/rknn_model_zoo/tree/main/models/CV/object_detection/yolo



## Arm Linux Demo

### Compiling and Building

The 'build-linux.sh' can be used for compiling demo for target including RV1106 and RV1103.

Changing the cross compiler path via the setting the `RK_RV1106_TOOLCHAIN`, shown as below:

```sh
export RK_RV1106_TOOLCHAIN=~/opts/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf
```

then, run the script：

```sh
./build-linux.sh -t <TARGET_PLATFORM> -b Release
```

Note: The RV1106 and RV1103 requires this 'arm-rockchip830-linux-uclibcgnueabihf' compiler to build the demo or another applications. 

### Push build output files to the board

Connecting the usb port to the PC, and pushing all demo folder to the directory '/userdata':

```sh
adb push install/rknn_yolov5_demo_Linux /userdata/
```

### Running 

```sh
adb shell
cd /userdata/rknn_yolov5_demo_Linux/

export LD_LIBRARY_PATH=/userdata/rknn_yolov5_demo_Linux/lib
./rknn_yolov5_demo model/RV1106/yolov5s-640-640.rknn model/bus.jpg
```

Note: 

- LD_LIBRARY_PATH must use the full path
- For performance reasons, the output fmt of the RKNN model is set to **RKNN_QUERY_NATIVE_NHWC_OUTPUT_ATTR** in the demo to obtain better inference performance. At this time, the model output buf is arranged in the order of NHWC. For example, the original shape of the first output is **1,255,80,80**. At this case, the shape output by RKNN is 1,80,80,255. The post-processing in this demo is also optimized and adjusted according to this order.
- RV1103B and RV1106B do not support NHWC output yet. To get better performance, we directly support post-processing of NC1HWC2 arrangement.
