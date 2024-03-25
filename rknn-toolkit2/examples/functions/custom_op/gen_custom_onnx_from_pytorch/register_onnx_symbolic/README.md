# Register onnx symbolic

本教程基于 pytorch-2.2.0，torchvision-0.16.0 版本进行测试验证。使用不同的版本可能会有差异。



## 1.使用场景

当 pytorch 已有算子转 onnx 失败时，例如 torch.nn.threshold(https://pytorch.org/docs/stable/generated/torch.nn.Threshold.html#torch.nn.Threshold)，可以参考本例程，为 pytorch op 添加转 onnx 的支持。

若需要自定义 pytorch op，请参考[文档](../register_pytorch_op/README.md)。



## 2.执行例子

```
python test.py
```



## 3. 代码解释

当模型存在 torch.nn.Threshold 算子，尝试导出onnx模型时会报错，具体情况如下:

```python
import torch

class Model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.m = torch.nn.Threshold(0.1, 20)

    def forward(self, x):
        y = self.m(x)
        return y

#导出 onnx 模型
x = torch.randn(1, 3, 10, 10)
m = Model()
torch.onnx.export(m, (x,), "threshold.onnx", opset_version=12)
```

此时会碰到报错:

```python
raise errors.SymbolicValueError(
torch.onnx.errors.SymbolicValueError: Unsupported: ONNX export of operator threshold, non-zero threshold. Please feel free to request support or submit a pull request on PyTorch GitHub: https://github.com/pytorch/pytorch/issues 
```



可以看到当前 torch2onnx 不支持 **threshold** 算子到 onnx 导出，导致转模型失败。对于这种情况，可以在 torch.onnx.export 前，注册 onnx 的导出规则。

- 首先我们找到  **torch.nn.threshold** 的输入定义，后续需要使用 parse_args 为每个输入配置对应的数据类型。

```python
# 在 torch.nn.functional.py 文件可以找到 threshold 的输入定义
result = _VF.threshold(input, threshold, value)
return result
```

- 定义一个 op 映射函数(对应下面代码中的 threshold_symbolic)，构建 pytorch、onnx 算子输入的对应关系，以下代码分别展示了三种注册方式，以及对应的区别

```python
from torch.onnx.symbolic_helper import parse_args

'''
需要注意，该op映射函数必须使用 parse_args 进行修饰，修饰每一个输入的数据类型，根据torch的代码，有以下定义:
Args:
    arg_descriptors: list of str, where each element is
        a string that specifies the type to convert to. Valid descriptors:
        "v": no conversion, keep torch._C.Value.
        "i": int
        "is": list of int
        "f": float
        "fs": list of float
        "b": bool
        "s": str
        "t": torch.Tensor
        "none": the variable is unused
'''

'''
自定义op的命名规范为: <生成组织>:<自定义op的名称>，例如以下例子是 rknn_cst::cst_threshold
'''

'''
例子一
不需要记录op属性时，可以忽略这些输入，在 g.op 函数中只记录 tensor输入
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
    output = g.op("rknn_cst::cstThreshold", x)
    return output


'''
例子二
当需要记录op属性时，必须在 g.op 中声明，声明方式为添加参数关键字，请注意参数关键字的结尾需要用 _{type} 声明数据类型，这里由于每一个参数都是浮点型，故末尾加上 _f 的数据类型声明
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
    output = g.op("rknn_cst::cstThreshold", x, threshold_f=threshold, value_f=value)
    return output

'''
例子三
若想要将属性记录为list，而不是单值，可采用下面方式
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
	output = g.op("rknn_cst::cstThreshold", x, param_f=[threshold,value]) 
	return output
```

- 声明以上函数后，将此函数注册到 torch2onnx 功能

```python
register_custom_op_symbolic("aten::threshold", threshold_symbolic, 12)
#第一个参数，"aten::threshold" ，必须和pytorch已有的定义匹配上，不可以随意变更
#第二个参数是前面定义的op映射函数
#第三个参数是配置的 onnx-opset 版本，目前可不管，填入常用的 opset version 即可
```



## 4.拓展

目前 torch2onnx 的自定义功能，不仅支持定义单个 torch op 到 单个 onnx op 的映射，理论上也支持下面形式

- 覆盖已有的 torch2onnx 导出规则
- 导出带有子图形式的自定义op

由于使用场景较少，该教程暂不提供这类复杂用法，若有需要请参考 onnx 的官方文档自行实现



