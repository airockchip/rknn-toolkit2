# Export custom onnx based on Pytorch

This example mainly explains how to export an ONNX model with a custom op under the Pytorch framework.



## Applicable scene

1. Solve the problem that Pytorch op cannot be converted into onnx op, such as torch.nn.threshold operator

2. For performance, convert multiple fragmented Pytorch ops into a single onnx op, and use neon/gpu to build the code implementation

3. Convert any function into a single onnx op, such as converting post-processing into an onnx op



## Introduction

When using torch.onnx.export to export an onnx model, a torch.jit.trace operation will be performed on the model first to convert it into a jit model, and then the op defined in the jit format will be mapped and converted into an onnx model. Therefore, there are two situations when implementing a custom operator:

- **Custom onnx op**. When the existing operators of pytorch do not support switching to onnx, such as the torch.nn.threshold operator, we can adopt this method. At this time, it only involves the process of converting jit model to onnx model, which is relatively simple. Please refer to [doc](./register_onnx_symbolic/README_EN.md)

- **Custom jit op and onnx op both**. Under normal circumstances, due to the high completeness of pytorch operators, complex operations can be split into fragmented underlying ops for support. However, the disadvantage is that the model may contain a lot of fragmented jit op definitions, resulting in poor performance. If you want to merge several jit ops into one and convert them into a single onnx op, please refer to [doc](./register_pytorch_op/README_EN.md)

