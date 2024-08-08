# ONNX Edit

RKNN-Toolkit2 has a built-in `rknn.utils.onnx_edit` tool. Used to adjust the onnx model. For some **onnx models with special structures**, **better inference performance** can be obtained after the adjusted model is converted to RKNN. The interface is defined as follow:

```python
onnx_edit(model: str,
          export_path : str,
          inputs_transform : dict = {},
          outputs_transform : dict = {}, 
          dataset: Optional[str] = None):
"""
    Args:
        model: The path to the onnx file.
        export_path: The path to the exported onnx file.
        inputs_transform: The dictionary of inputs transform equation.
        outputs_transform: The dictionary of outputs transform equation.
        dataset: The path to the dataset file. dataset file should meet rknn.build requirements. If given, transform of inputs will also apply on dataset and generate a new dataset on the same folder of export_path.
"""
```

Currently the following functions are available:

- Reshape and transpose adjustments to input and output（Modification of dynamic shapes is currently not supported.）



## 1.Reshape and transpose adjustments to input and output

If some models can reshape and transpose the input and output to a certain extent before converting to RKNN, the graph optimization function of RKNN-Toolkit2 can be used to make the final RKNN model more concise and **have better inference performance**. For example, by executing the `test.py` script in this directory, you can compare the differences in the RKNN models generated before and after onnx_edit.

Transformation equation rules:

- It must be composed of two parts of symbolic shape, separated by **'->'**. The **left side is the original input symbolic shape**, and the **right side is the transformed symbolic shape**.

- The character in symbolic shape are only allowed to be **[a-z]** or **','** **'1'**. Other numeric shape characters are not supported except for '1'. The reason is that the same numeric characters cannot determine the context of transpose.

- Characters **[a-z]** are considered independent and have no order relationship, that means, **'a,b->b,a'** and **'c,a->a,c'** represent the same transformation.

- Separated by **','**, **original input symbolic shape** on the left side of the equation must **match the dimensions of the input**. For example, the original input definition is **[32,4,1,64]**, and the input characters can be **'a,b,c,d'** or **'a,b,1,d'**

- **Every character** of the original input symbolic shape, except '1', **must exist in the transformed symbolic shape**. For example, **'a,1,c,d->ac,d,1'** is valid, **'a,1, c,d->ac,1'** is invalid because **'d'** is missing.

- The transformed symbolic shape allows the insertion of **any number of '1'** to achieve a **dimension expansion**, such as **'a,b,c-> a,1,cb,1,1'**

- The original input symbolic shape allows using **multiple characters** and **assignment formulas** to split the shape. For example, the original input definition is **[32,4,1,64]**, **'ab,c,d,qk[a=2,k =8]->aq,cd,1,kb'**, which means split 32 into 2x16, split 64 into 8x8, and then perform transpose and reshape operations. The part '[ ]' is called the assignment formula, and multiple formulas are separated by **','**. It is allowed that one character in the split is not assigned a value. At this time, the corresponding shape will be automatically inferred. For example, the assignment formula only gives **a=2**, and **ab=32** is known, **then b=16 is automatically inferred**; if the inferred shape is abnormal An error will be reported directly, for example, ab=32. If a=5 is assigned, then b=6.4. The dimension must be an integer, and an exception error will be thrown.

- Support dynamic shape
- If the dimensions of a dynamic shape are split, only one of the split dimensions is allowed to be a dynamic shape. Assuming there are 3-dimensional dynamic shapes, `ab,c,d->a,c,d,b` is invalid , because both a and b are unknown dimensions and cannot be split. And `ab,c,d[a=2]->a,c,d,b` is valid
- Allows assignment of dimensions to dynamic shapes, in which case the corresponding dimensions will become fixed shapes. Assuming there is a 3-dimensional dynamic shape, `ab,c,d[a=2]->a,c,d,b` will cause the changed dimensions to be [2,c,d,b], and the first dimension will be 2, and `ab,c,d[a=2,b=5,c=7,d=11]->a,c,d,b` will make the dimension fixed to [2,7,11,5].




Example of transform equation:

```shell
1. Reshape 3D to 4D:                'a,b,c->a,b,1,c' or 'a,b,c->a,1,b,c'
2. Reshape 5D to 4D:                'a,b,c,d,e->ab,c,d,e' or 'a,b,c,d,e->a,bce,1,1'
3. Transpose(0,3,1,2):              'a,b,c,d->a,d,b,c'
4. Transpose, merge dim:            'a,b,c,d->ad,c,b' or 'a,b,c,d->d,acb,1'
5. Split dim, transpose, merge dim: 'a,bc,de,f[b=2,d=4]->ab,fe,dc,1',  		#  'c','e' will be auto infered
6. Split dim, transpose, merge dim: 'a,bc,de,f[b=2,c=24,d=4]->ab,fe,dc,1',  #  'bc' = 24 shuold be equal to tensor's 2nd-dim, 'e' will be auto infered
```



After running `test.py`, onnx_edit interface will show transform log as follow:

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

