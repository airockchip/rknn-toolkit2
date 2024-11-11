# Zero-copy examples

This project shows how to implement zero-copy technology on different platforms to improve the efficiency of model inference. The specific example code is located in the examples/rknn_api_demo and examples/RV1106_RV1103 directories.

## Zero-copy examples for non-RV1103 and RV1106 platforms

The following examples show various ways to use zero-copy technology on non-RV1103 and RV1106 platform series. The code can be found in examples/rknn_api_demo:

1. rknn_create_mem_demo: This example shows how to use the rknn_create_mem interface to create zero-copy operations for input/output.

2. rknn_create_mem_with_rga_demo: This example shows how to use the rknn_create_mem interface to create zero-copy operations for input/output and combine it with RGA for scaling.
3. rknn_set_internal_mem_from_fd_demo: This example uses the MMZ library to allocate the model's input, output, weights, and internal memory, and implements zero-copy operations through the rknn_create_mem_from_fd interface.
4. rknn_with_mmz_demo: This example uses the MMZ library to allocate the model's input/output memory, and implements zero-copy operations through the rknn_create_mem_from_phys interface.

## Zero-copy examples for RV1103 and RV1106 platforms

On the RV1103 series and RV1106 series platform series, this project provides the following zero-copy examples, see examples/RV1106_RV1103 for the code:
1. rknn_mobilenet_demo: This example shows the process of using zero-copy technology to perform MobileNet model inference on the RV1103 series and RV1106 series platforms, where main_nhwc.c only supports RV1103 and RV1106.
2. rknn_yolov5_demo: This example demonstrates the process of using zero-copy technology to perform YOLOv5 model inference on RV1103 and RV1106 platforms.