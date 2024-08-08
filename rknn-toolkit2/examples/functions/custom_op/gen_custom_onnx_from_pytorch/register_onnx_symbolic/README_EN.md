# Register onnx symbolic

This tutorial is based on pytorch-2.2.0 and torchvision-0.16.0 versions for testing and verification. There may be differences using different versions.



## 1.Usage scenarios

When pytorch fails to convert existing operators to onnx, such as torch.nn.threshold (https://pytorch.org/docs/stable/generated/torch.nn.Threshold.html#torch.nn.Threshold), you can refer to this tutorial, adds support for converting pytorch op to onnx op.

If you need to customize pytorch op, please refer to [doc](../register_pytorch_op/README_EN.md).



## 2.Usage

```
python test.py
```



## 3.Code explanation

When the model has the torch.nn.Threshold operator, an error will be reported when trying to export the onnx model. The specific situation is as follows:

```python
import torch

class Model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.m = torch.nn.Threshold(0.1, 20)

    def forward(self, x):
        y = self.m(x)
        return y

x = torch.randn(1, 3, 10, 10)
m = Model()
torch.onnx.export(m, (x,), "threshold.onnx", opset_version=12)
```

At this time we encounter an error:

```python
raise errors.SymbolicValueError(
torch.onnx.errors.SymbolicValueError: Unsupported: ONNX export of operator threshold, non-zero threshold. Please feel free to request support or submit a pull request on PyTorch GitHub: https://github.com/pytorch/pytorch/issues 
```



It can be seen that currently torch2onnx does not support the export of the **threshold** operator to onnx, causing the model transfer to fail. For this case, you can register the export rules of onnx before torch.onnx.export.

- First we find the input definition of **torch.nn.threshold**, and then we need to use parse_args to configure the corresponding data type for each input.

```python
# The input definition of threshold can be found in the torch.nn.functional.py 
result = _VF.threshold(input, threshold, value)
return result
```

- Define an op mapping function (corresponding to threshold_symbolic in the code below) and construct the corresponding relationship between pytorch and onnx operator input. The following code shows the three registration methods and the corresponding differences.

```python
from torch.onnx.symbolic_helper import parse_args

'''
It should be noted that the op mapping function must be modified with parse_args to modify each input data type. According to the torch code, it has the following definition:
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
The naming convention for custom ops is: <Generation organization>:<Name of custom op>, for example, the following example is rknn_cst::cst_threshold
'''

'''
Case-1
When you do not need to record the op attribute, you can ignore these inputs and only record the tensor input in the g.op function.
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
    output = g.op("rknn_cst::cstThreshold", x)
    return output


'''
Case-2
When the op attribute needs to be recorded, it must be declared in g.op. The declaration method is to add the parameter keyword. Please note that the data type needs to be declared with _{type} at the end of the parameter keyword. Since each parameter is a floating point type , so the data type declaration of _f is added at the end
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
    output = g.op("rknn_cst::cstThreshold", x, threshold_f=threshold, value_f=value)
    return output

'''
Case-3
If you want to record attributes as a list instead of a single value, you can use the following method
'''
@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
	output = g.op("rknn_cst::cstThreshold", x, param_f=[threshold,value]) 
	return output
```

- After declaring the above function, register this function to the torch2onnx function

```python
register_custom_op_symbolic("aten::threshold", threshold_symbolic, 12)
#The first parameter, "aten::threshold", must match the existing op definition of pytorch and cannot be changed at will.
#The second parameter is the op mapping function defined earlier
#The third parameter is the configured onnx-opset version. You can ignore it for now. Just fill in the commonly used opset version.
```



## 4.Extra

Currently, the custom function of torch2onnx not only supports defining the mapping from a single torch op to a single onnx op, but also theoretically supports the following forms

- Overwrite existing torch2onnx export rules
- Export custom ops with subgraph form

Due to the small number of usage scenarios, this tutorial does not provide such complex usage. If necessary, please refer to the official documentation of onnx to implement it yourself.



