# Register Pytorch op

建议使用 Pytorch 1.13 或 2.x 版本执行本示例，且 python 环境中安装有 ninja 库



## 1.使用场景

当我们需要把模型中的一整块结构/复杂函数进行合并，并转换成 ONNX 的一个 op 时，可以参考本示例实现。

- 此功能的实现依赖于 torch.utils.cpp_extension 模块，目前参考文档较少，可以参考
  - 自定义op的文档： https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/core/op_registration/README.md
  - 自定义op单元测试：https://github.com/pytorch/pytorch/blob/main/test/test_cpp_extensions_jit.py
- 请注意，pytorch 自定义 op 有多种实现方式，这里仅介绍其中一种较为方便的实现方式



完整使用流程如下：

1）定义算子的 cpp 代码（若只用于导出模型，不在推理/反向传播中使用自定义功能，则cpp代码的计算结果正确与否不影响后续流程，只要输入输出的定义、shape能正确即可）

2）使用 torch.utils.cpp_extension 加载 cpp 定义的自定义算子

3）使用自定义算子构造模型（或替换原模型中对应的结构）

4）添加算子到 onnx 的映射函数

5）导出 onnx 模型



## 2.执行例子

```
python test.py
```



## 3.代码解释

1）定义算子的 cpp 代码

假设有 tensor A 和 tensor B，它们的 shape 相同，构建一个自定义 op ，其输入为（A，B，alpha），输出为（A*alpha -B，B\*alpha - A），此时 cpp 定义代码如下

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

其中的`static auto registry = torch::RegisterOperators().op("cst::dual_residual", &dual_residual);` 语句，作用是声明自定义op在 python 接口中的名称，例如示例代码执行后，对应的 python 接口为 torch.ops.cst.dual_residual



2）使用 torch.utils.cpp_extension 接口加载上一步骤定义的 cpp 代码

```python
torch.utils.cpp_extension.load_inline(
        name="test",
        cpp_sources=cpp_source,
        functions="dual_residual",
        verbose=True,
        is_python_module=True,
    )
# functions的字符串输入 dual_residual 需要和 cpp 定义中的函数名字一致
```



3）使用自定义算子构造模型

```python
class Fake_model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.const = .5

    def forward(self, x, y):
        r1, r2 = torch.ops.cst.dual_residual(x, y, self.const)
        return r1, r2
```



4）参考[文档](../register_onnx_symbolic/README.md)，添加自定义 torch op 到 onnx op 的映射

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



5）导出模型

```python
torch.onnx.export(m, (x, y), "dual_residual.onnx", opset_version=12)
```

