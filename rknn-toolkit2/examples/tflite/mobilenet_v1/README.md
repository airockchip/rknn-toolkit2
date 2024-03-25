# TFLite MobileNet V1

## Model Source
The model used in this example come from the following open source projects:  
https://www.tensorflow.org/lite/guide/hosted_models?hl=zh-cn

## Script Usage
*Usage:*
```
python test.py
```
*rknn_convert usage:*
```
python3 -m rknn.api.rknn_convert -t rk3568 -i ./model_config.yml -o ./
```
*Description:*
- The default target platform in script is 'rk3566', please modify the 'target_platform' parameter of 'rknn.config' according to the actual platform.
- If connecting board is required, please add the 'target' parameter in 'rknn.init_runtime'.

## Expected Results
This example will print the TOP5 labels and corresponding scores of the test image classification results, as follows:
```
-----TOP 5-----
[ 156] score:0.928223 class:"Shih-Tzu"
[ 155] score:0.063171 class:"Pekinese, Pekingese, Peke"
[ 205] score:0.004299 class:"Lhasa, Lhasa apso"
[ 284] score:0.003096 class:"Persian cat"
[ 285] score:0.000171 class:"Siamese cat, Siamese"
```
- Note: Different platforms, different versions of tools and drivers may have slightly different results.
