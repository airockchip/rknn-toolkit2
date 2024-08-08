# 基于 Pytorch 导出带 custom op onnx 模型

本示例主要说明如何在 Pytorch 的框架下，导出带有自定义op的ONNX模型



## 适用场景

1.解决Pytorch op无法转成 onnx op的问题，例如 torch.nn.threshold 算子

2.为了性能，将多个零碎的Pytorch op转成单个 onnx op，并使用 neon/gpu 构建代码实现

3.将任意函数转为单个 onnx op，例如将后处理转为 op



## 原理介绍

使用 torch.onnx.export 导出 onnx 模型时，会先对 model 做一次 torch.jit.trace 操作，转成 jit 模型，随后对 jit 格式定义的 op 进行映射，转成 onnx 模型。故实现自定义算子，存在以下两种情况:

- **自定义 onnx op**。当 pytorch 已有的算子不支持转到 onnx，比如 torch.nn.threshold 算子，我们可以采取这种方式，此时只涉及 jit model -> onnx model 的链路，比较简单。请参考[文档](./register_onnx_symbolic/README.md)

- **同时自定义 jit op 以及 onnx op**。正常情况下，由于 pytorch 算子的完备性很高，复杂操作都可以被拆成零碎的底层 op 进行支持，几乎不存在需要自己自定义 jit op 的情况。但缺点是模型有可能包含了非常多零碎的 jit op定义，性能较差，若想把某几个 jit op 合并成一个，并转为一个单一的 onnx op，请参考[文档](./register_pytorch_op/README.md)

