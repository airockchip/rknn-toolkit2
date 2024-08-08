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
[155] score:0.992188 class:"Shih-Tzu"
[154] score:0.002636 class:"Pekinese, Pekingese, Peke"
[204] score:0.002636 class:"Lhasa, Lhasa apso"
[283] score:0.001698 class:"Persian cat"
[896] score:0.000273 class:"washbasin, handbasin, washbowl, lavabo, wash-hand basin"

input shape: 1,3,160,160
W The input[0] need NHWC data format, but NCHW set, the data format and data buffer will be changed to NHWC.
-----TOP 5-----
[155] score:0.558594 class:"Shih-Tzu"
[154] score:0.408447 class:"Pekinese, Pekingese, Peke"
[204] score:0.031036 class:"Lhasa, Lhasa apso"
[194] score:0.000956 class:"Dandie Dinmont, Dandie Dinmont terrier"
[219] score:0.000256 class:"cocker spaniel, English cocker spaniel, cocker"

input shape: 1,3,256,256
W The input[0] need NHWC data format, but NCHW set, the data format and data buffer will be changed to NHWC.
-----TOP 5-----
[155] score:0.980957 class:"Shih-Tzu"
[154] score:0.008835 class:"Pekinese, Pekingese, Peke"
[204] score:0.004883 class:"Lhasa, Lhasa apso"
[193] score:0.000929 class:"Australian terrier"
[200] score:0.000509 class:"Tibetan terrier, chrysanthemum dog"
```
- Note: Different platforms, different versions of tools and drivers may have slightly different results.
