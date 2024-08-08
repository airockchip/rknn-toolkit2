# Example of resnet18

## Model Source

### Original model
The models used in this example come from the torchvision project:
https://github.com/pytorch/vision/tree/main/torchvision/models

### Convert to RKNN model
Please refer to the example in the RKNN Toolkit2 project to generate the RKNN model:
https://github.com/rockchip-linux/rknn-toolkit2/tree/master/examples/pytorch/resnet18

## Script Usage

Usage

```
python test.py
```

## Expected results

This example will print the TOP5 labels and corresponding scores of the test image classification results. For example, the inference results of this example are as follows:
```
-----TOP 5-----
[812] score:0.999680 class:"space shuttle"
[404] score:0.000249 class:"airliner"
[657] score:0.000013 class:"missile"
[466] score:0.000009 class:"bullet train, bullet"
[895] score:0.000008 class:"warplane, military plane"
```

1. The label index with the highest score is 812, the corresponding label is `space shuttle`.
2. Different platforms, different versions of tools and drivers may have slightly different results.
