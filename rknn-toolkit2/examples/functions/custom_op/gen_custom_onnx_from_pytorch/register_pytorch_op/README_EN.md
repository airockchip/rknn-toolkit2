# Register Pytorch op

It is recommended to use Pytorch 1.13 or 2.x version to execute this example, and the ninja library is installed in the python environment



## 1.Usage scenarios

When we need to merge an entire structure/complex function in the model and convert it into an ONNX op, we can refer to this example implementation.

- The implementation of this function depends on the torch.utils.cpp_extension module. There are currently few reference documents, so you can refer to
  - Custom op documentation: https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/core/op_registration/README.md
  - Custom op unit test: https://github.com/pytorch/pytorch/blob/main/test/test_cpp_extensions_jit.py
- Please note that there are many ways to implement pytorch custom op. Here we only introduce one of the more convenient ways to implement it.



The complete usage process is as follows:

1) Define the cpp code of the operator (if it is only used to export the model and does not use custom functions in inference/backpropagation, the correctness of the calculation results of the cpp code will not affect the subsequent process, as long as the definition and shape of the input and output are correct)

2) Use torch.utils.cpp_extension to load the custom operator defined in cpp

3) Use custom operators to construct the model (or replace the corresponding structure in the original model)

4) Add operator to onnx mapping function

5) Export onnx model



## 2.Usage

```
python test.py
```



## 3.Code explanation

1）Define operator in cpp code.

Assume that there are tensor A and tensor B with the same shape. Build a custom op wiht input as (A, B, alpha) and the output as (A*alpha -B, B\*alpha - A). The definition code is as follows

```cpp
cpp_source = """
#include <torch/script.h>
std::tuple<torch::Tensor, torch::Tensor> dual_residual(torch::Tensor x, torch::Tensor y, double_t alpha) {
    torch::Tensor tmp1 = x* alpha - y;
    torch::Tensor tmp2 = y* alpha - x;
    return std::tuple<torch::Tensor, torch::Tensor>(tmp1, tmp2);}
static auto registry = torch::RegisterOperators()
    .op("cst::dual_residual", &dual_residual);
"""
```

The `static auto registry = torch::RegisterOperators().op("cst::dual_residual", &dual_residual);` statement is used to declare the name of the custom op in the python interface. For example, after the example code is executed, the corresponding The python interface is torch.ops.cst.dual_residual



2）Use the torch.utils.cpp_extension interface to load the cpp code defined in the previous step

```python
torch.utils.cpp_extension.load_inline(
        name="test",
        cpp_sources=cpp_source,
        functions="dual_residual",
        verbose=True,
        is_python_module=True,
    )
# The string input dual_residual of functions needs to be consistent with the function name in the cpp definition
```



3）Construct a model using custom operators

```python
class Model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.const = .5

    def forward(self, x, y):
        r1, r2 = torch.ops.cst.dual_residual(x, y, self.const)
        return r1, r2
```



4）Refer to [doc](../register_onnx_symbolic/README_EN.md) to add a custom torch op to onnx op mapping

```python
from torch.onnx import register_custom_op_symbolic
from torch.onnx.symbolic_helper import parse_args

@parse_args('v', 'v', 'f')
def dual_residual_symbolic(g, x, y, alpha):
    output = g.op("rknn_cst::cstDualResidual", x, y, alpha_f=alpha, 
                outputs=2)  
    return output

register_custom_op_symbolic("cst::dual_residual", dual_residual_symbolic, 12)
```



5）export onnx model

```python
torch.onnx.export(m, (x, y), "dual_residual.onnx", opset_version=12)
```

