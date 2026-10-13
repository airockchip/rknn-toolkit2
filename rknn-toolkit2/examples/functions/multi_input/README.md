# How to convert multi-input model

## Model Source
The model used in this example come from 'gen_pb.py'.

## Script Usage
To regenerate the frozen model and input arrays, run:
```
python gen_pb.py
```
The generator uses TensorFlow 1.x graph APIs and selects `tf.compat.v1` when running with TensorFlow 2.x.
The generated `input2.npy` and `input3.npy` use NCHW layout, as required by RKNN calibration and inference.

*Usage:*
```
python test.py
```
*Description:*
- The default target platform in script is 'rk3566', please modify the 'target_platform' parameter of 'rknn.config' according to the actual platform.
- If connecting board is required, please add the 'target' parameter in 'rknn.init_runtime'.
- The model has multiple inputs, so multiple input files need to be set in 'dataset.txt':
    ```
    dog_128x128.jpg input2.npy input3.npy dog_128x128_gray.png
    ```
- The 'inputs' and 'data_format' of 'rknn.inference' should also be set accordingly, as follows:
    ```
    rknn.inference(inputs=[img, input2, input3, img_gray], data_format=['nhwc', 'nchw', 'nchw', 'nhwc'])
    ```
## Expected Results
This example will print outputs of inference, as follows:
```
inference result:  [array([[ -3.1041162,  -7.0548096,  -5.361655 , ..., -14.391811 ,
        -15.802773 , -11.287695 ]], dtype=float32)]
```
- Note: Different platforms, different versions of tools and drivers may have slightly different results.
