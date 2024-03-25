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
[812]: 0.999676 [class: space shuttle]
[404]: 0.000249 [class: airliner]
[657]: 0.000014 [class: missile]
[833]: 0.000009 [class: submarine, pigboat, sub, U-boat]
[466]: 0.000009 [class: bullet train, bullet]
```

1. The label index with the highest score is 812, the corresponding label is `space shuttle`.
2. Different platforms, different versions of tools and drivers may have slightly different results.
