# CHANGELOG

## v2.0.0-beta0

 - Support RK3576 (Beta)
 - Support RK2118 (Beta)
 - Support SDPA (Scaled Dot Product Attention) to improve transformer performance
 - Improve custom operators support
 - Improve MatMul API
 - Improve support for Reshape,Transpose,BatchLayernorm,Softmax,Deconv,Matmul,ScatterND etc.
 - Support pytorch 2.1
 - Improve support for QAT models of pytorch and onnx
 - Optimize automatic generation of C++ code



## v1.6.0

 - Support ONNX model of OPSET 12~19
 - Support custom operators (including CPU and GPU)
 - Improve support for dynamic weight convolution, Layernorm, RoiAlign, Softmax, ReduceL2, Gelu, GLU, etc.
 - Added support for python3.7/3.9/3.11
 - Add rknn_convert function
 - Improve transformer support
 - Improve MatMul API, such as increasing the K limit length, RK3588 adding int4 * int4 -> int16 support, etc.
 - Reduce RV1106 rknn_init initialization time, memory consumption, etc.
 - RV1106 adds int16 support for some operators
 - Fixed the problem that the convolution operator of RV1106 platform may make random errors in some cases.
 - Improve user manual
 - Reconstruct the rknn model zoo and add support for multiple models such as detection, segmentation, OCR, and license plate recognition.



## v1.5.2

- Improve dynamic shape support
- Improve matmul api support
- Add GPU back-end implementations for some operators such as matmul
- Improve transformer support
- Reduce rknn_init memory usage
- Optimize rknn_init time-consuming



## v1.5.0

- Support RK3562
- Support more NPU operator fuse, such as Conv-Silu/Conv-Swish/Conv-Hardswish/Conv-sigmoid/Conv-HardSwish/Conv-Gelu ..
- Improve support for  NHWC output layout
- RK3568/RK3588：The maximum input resolution up to 8192
- Improve support for Swish/DataConvert/Softmax/Lstm/LayerNorm/Gather/Transpose/Mul/Maxpool/Sigmoid/Pad
- Improve support for CPU operators (Cast, Sin, Cos, RMSNorm, ScalerND, GRU)
- Limited support for dynamic resolution
- Provide MATMUL API
- Add RV1103/RV1106 rknn_server application as proxy between PC and board
- Add more examples such as rknn_dynamic_shape_input_demo and video demo for yolov5
- Bug fix



## v1.4.0

- Support more NPU operators, such as Reshape、Transpose、MatMul、 Max、Min、exGelu、exSoftmax13、Resize etc.

- Add **Weight Share**  function, reduce memory usage.

- Add **Weight Compression** function, reduce memory and bandwidth usage.(RK3588/RV1103/RV1106)

- RK3588 supports storing weights or feature maps on SRAM, reducing system bandwidth consumption.

- RK3588 adds the function of running a single model on multiple cores at the same time.

- Add new output layout NHWC (C has alignment restrictions) .

- Improve support for non-4D input.

- Add more examples such as rknn_yolov5_android_apk_demo and rknn_internal_mem_reuse_demo.

- Bug fix.

    

## v1.3.0

- Support RV1103/RV1106（Beta SDK）
- rknn_tensor_attr support w_stride(rename from stride) and h_stride
- Rename rknn_destroy_mem()
- Support more NPU operators, such as Where, Resize, Pad, Reshape, Transpose etc.
- RK3588 support multi-batch multi-core mode
- When RKNN_LOG_LEVEL=4, it supports to display the MACs utilization and bandwidth occupation of each layer.
- Bug fix



## v1.2.0

- Support RK3588
- Support more operators, such as GRU、Swish、LayerNorm etc.
- Reduce memory usage
- Improve zero-copy interface implementation
- Bug fix



## v1.1.0

- Support INT8+FP16 mixed quantization to improve model accuracy
- Support specifying input and output dtype, which can be solidified into the model
- Support multiple inputs of the model with different channel mean/std
- Improve the stability of multi-thread + multi-process runtime
- Support flashing cache for fd pointed to internal tensor memory which are allocated by users
- Improve dumping internal layer results of the model
- Add rknn_server application as proxy between PC and board
- Support more operators, such as HardSigmoid、HardSwish、Gather、ReduceMax、Elu
- Add LSTM support (structure cifg and peephole are not supported, function: layernormal, clip is not supported)
- Bug fix



## v1.0

- Optimize the performance of rknn_inputs_set()
- Add more functions for zero-copy
- Add new OP support, see OP support list document for details.
- Add multi-process support
- Support per-channel quantitative model
- Bug fix



## v0.7

- Optimize the performance of rknn_inputs_set(), especially for models whose input width is 8-byte aligned.

- Add new OP support, see OP support list document for details.

- Bug fix

    

## v0.6
- Initial version

