# Caffe VGG-SSD

## Model Source
The model used in this example come from the following open source projects:  
https://github.com/weiliu89/caffe/tree/ssd#models  
But the model download link has expired, please download from [VGG_VOC0712_SSD_300x300_iter_120000.caffemodel](https://ftzr.zbox.filez.com/v2/delivery/data/95f00b0fc900458ba134f8b180b3f7a1/asset/vgg-ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel).

## Script Usage
*Usage:*
```
python test.py
```
*rknn_convert usage:*
```
python3 -m rknn.api.rknn_convert -t rk3566 -i ./model_config.yml -o ./
```
*Description:*
- The default target platform in script is 'rk3566', please modify the 'target_platform' parameter of 'rknn.config' according to the actual platform.
- If connecting board is required, please add the 'target' parameter in 'rknn.init_runtime'.

## Expected Results
This example will save the result of object detection to the 'result.jpg', as follows:  
![result](result_truth.jpg)
- Note: Different platforms, different versions of tools and drivers may have slightly different results.