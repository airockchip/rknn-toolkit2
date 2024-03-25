# How to use dynamic shape function

## Model Source
The model used in this example come from the following open source projects:  
https://github.com/shicai/MobileNet-Caffe

### Convert to RKNN model
Please refer to the example in the RKNN Toolkit2 project to generate the RKNN model:
https://github.com/rockchip-linux/rknn-toolkit2/tree/master/examples/functions/dynamic_shape

## Script Usage
*Usage:*
```
python test.py
```

## Expected Results
This example will print the TOP5 labels and corresponding scores of the test image classification results for each different input shape, as follows:
```
model: mobilenet_v2

input shape: 1,3,224,224
W The input[0] need NHWC data format, but NCHW set, the data format and data buffer will be changed to NHWC.
-----TOP 5-----
[155] score:0.936035 class:"Shih-Tzu"
[204] score:0.002516 class:"Lhasa, Lhasa apso"
[154] score:0.002172 class:"Pekinese, Pekingese, Peke"
[283] score:0.001601 class:"Persian cat"
[284] score:0.000286 class:"Siamese cat, Siamese"

input shape: 1,3,160,160
W The input[0] need NHWC data format, but NCHW set, the data format and data buffer will be changed to NHWC.
-----TOP 5-----
[155] score:0.606934 class:"Shih-Tzu"
[154] score:0.329834 class:"Pekinese, Pekingese, Peke"
[204] score:0.025085 class:"Lhasa, Lhasa apso"
[194] score:0.001038 class:"Dandie Dinmont, Dandie Dinmont terrier"
[219] score:0.000241 class:"cocker spaniel, English cocker spaniel, cocker"

input shape: 1,3,256,256
W The input[0] need NHWC data format, but NCHW set, the data format and data buffer will be changed to NHWC.
-----TOP 5-----
[155] score:0.927246 class:"Shih-Tzu"
[154] score:0.007225 class:"Pekinese, Pekingese, Peke"
[204] score:0.004616 class:"Lhasa, Lhasa apso"
[193] score:0.000878 class:"Australian terrier"
[283] score:0.000482 class:"Persian cat"
```
- Note: Different platforms, different versions of tools and drivers may have slightly different results.
