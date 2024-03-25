# ONNX Edit

rknn.utils.onnx_edit 是 RKNN-Toolkit2 的内置工具，用于调整onnx模型。对于一些具有特殊结构的onnx模型，调整后的模型在转换为RKNN后可以获得更好的推理性能。接口定义如下：

```python
from rknn.utils import onnx_edit
onnx_edit(model: str,
          export_path : str,
          inputs_transform : dict = {},
          outputs_transform : dict = {}, 
          dataset: Optional[str] = None):
"""
    Args:
        model: The path to the onnx file.
        export_path: The path to the exported onnx file.
        inputs_transform: The dictionary of inputs transform equation. Feed like {'input_0': 'a,b,c->1,ab,1,c', ...}
        outputs_transform: The dictionary of outputs transform equation. Feed like {'output_2': 'a,b,c,->1,b,1,ac', ...}
        dataset: The path to the dataset file. dataset file should meet rknn.build requirements. If given, transform of inputs will also apply on dataset and generate a new dataset on the same folder of export_path.
"""
```

目前可以使用以下功能：

- 对输入和输出进行 reshape 和 transpose 调整（目前暂不支持对动态shape进行修改）



## 1.对输入和输出进行 reshape 和 transpose 调整

如果某些模型在转换为 RKNN 之前，能够对输入输出进行特定的 reshape 和 transpose 操作，则可以利用 RKNN-Toolkit2 的图优化功能，使最终的 RKNN 模型更加简洁，具有更好的推理性能。例如，通过执行该目录下的 test.py 脚本，可以比较 onnx_edit 前后生成的 RKNN 模型的差异。

变换方程书写规则：

- 必须有两部分字符组成，并用 ‘->’ 隔开，左边为原始字符shape，右边是变换后字符shape
- 字符shape里面的符号只允许是 [a-z] 或 ',' '1'，除了'1'以外，不支持其他数字shape字符，原因是相同数字字符无法判断 transpose 的前后关系
- 字符[a-z]认为是独立的，没有顺序关系，即 'a,b->b,a' 和 'c,a->a,c' 表示的是一样的变换
- 等式左边的原始字符shape，被 ',' 分隔成 n 份，n的个数必须和模型的维度匹配，例如原始输入定义是 [32,4,1,64], 输入字符可以是 'a,b,c,d' 或 'a,b,1,d'
- 原始字符shape的每一个符号，除了'1'，都必须存在于变换后字符shape，例如 'a,1,c,d->ac,d,1' 是有效的，'a,1,c,d->ac,1' 是无效的，缺了 'd'
- 变换后字符shape，允许插入任意个数的 '1'，达成扩维效果，例如 'a,b,c-> a,1,cb,1,1'
- 原始字符shape，允许用多个字母以及赋值公式来表示对shape进行拆分，例如原始输入定义是 [32,4,1,64]，'ab,c,d,qk[a=2,k=8]->aq,cd,1,kb'，表示将 32 拆分成 2x16，将 64 拆分成 8x8，再进行 transpose, reshape 操作。其中 '[ ]' 的部分称为赋值公式，多个公式用 ',' 符号分隔。此外，允许拆分中的某个字符没有赋值，此时会自动推断对应的shape，例如赋值公式只给了 a=2，已知在模型中 ab=32，则自动推断出 b=16；若推断出的shape异常会直接报错，比如 ab=32，若赋值 a=5，则 b=6.4，又维度必须是整数，此时会抛出异常错误。



以下是一些变换例子：

```shell
1. Reshape 3D to 4D:                'a,b,c->a,b,1,c' or 'a,b,c->a,1,b,c'
2. Reshape 5D to 4D:                'a,b,c,d,e->ab,c,d,e' or 'a,b,c,d,e->a,bce,1,1'
3. Transpose(0,3,1,2):              'a,b,c,d->a,d,b,c'
4. Transpose, merge dim:            'a,b,c,d->ad,c,b' or 'a,b,c,d->d,acb,1'
5. Split dim, transpose, merge dim: 'a,bc,de,f[b=2,d=4]->ab,fe,dc,1',  		#  'c','e' will be auto infered
6. Split dim, transpose, merge dim: 'a,bc,de,f[b=2,c=24,d=4]->ab,fe,dc,1',  #  'bc' = 24 shuold be equal to tensor's 2nd-dim, 'e' will be auto infered
```



执行 `test.py` 后，onnx_edit 接口会打印出以下变换的公式：

```shell
I For 'k_cache.1':'a,b,c,d->1,ad,b,c'
  Input:'k_cache.1' was reset as shape-[1, 32, 48, 1].(Origin shape is [4, 48, 1, 8])
  Insert ops to transform [1, 32, 48, 1] to [4, 48, 1, 8]:
  - Insert reshape op. [1, 32, 48, 1] reshape to [4, 8, 48, 1].
  - Insert transpose op. [4, 8, 48, 1] transpose(0, 2, 3, 1) to [4, 48, 1, 8].
I For 'k_cache':'a,b,c,d->1,ab,c,d'
  Output:'k_cache' was reset as shape-[1, 32, 48, 1].(Origin shape is [4, 8, 48, 1]) 
  Insert ops to transform [4, 8, 48, 1] to [1, 32, 48, 1]:
  - Insert reshape op. [4, 8, 48, 1] reshape to [1, 32, 48, 1].
```
