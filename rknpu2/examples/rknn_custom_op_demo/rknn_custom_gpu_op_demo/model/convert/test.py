import os
import urllib
import traceback
import time
import sys
import numpy as np
import cv2 as cv
from rknn.api import RKNN
from matplotlib import pyplot as plt

from rknn.api.custom_op import get_node_attr

from matplotlib import gridspec

import get_dataset_colormap

OUT_SIZE = 192

LABEL_NAMES = np.asarray([
    'background', 'person'  
])

FULL_LABEL_MAP = np.arange(len(LABEL_NAMES)).reshape(len(LABEL_NAMES), 1)
FULL_COLOR_MAP = get_dataset_colormap.label_to_color_image(FULL_LABEL_MAP)

def vis_segmentation(image, seg_map):
    plt.figure(figsize=(15, 5))
    grid_spec = gridspec.GridSpec(1, 4, width_ratios=[6, 6, 6, 1])

    plt.subplot(grid_spec[0])
    plt.imshow(image)
    plt.axis('off')
    plt.title('input image')

    plt.subplot(grid_spec[1])
    seg_image = get_dataset_colormap.label_to_color_image(
        seg_map, get_dataset_colormap.get_pascal_name()).astype(np.uint8)
    plt.imshow(seg_image)
    plt.axis('off')
    plt.title('segmentation map')

    plt.subplot(grid_spec[2])
    plt.imshow(image)
    plt.imshow(seg_image, alpha=0.7)
    plt.axis('off')
    plt.title('segmentation overlay')

    unique_labels = np.unique(seg_map)

    ax = plt.subplot(grid_spec[3])
    plt.imshow(FULL_COLOR_MAP[unique_labels].astype(np.uint8), interpolation='nearest')
    ax.yaxis.tick_right()
    plt.yticks(range(len(unique_labels)), LABEL_NAMES[unique_labels])
    plt.xticks([], [])
    ax.tick_params(width=0)

    # plt.show()
    # Save the figure to a file
    plt.savefig('result.jpg')



class ArgMax:
    op_type = "ArgMax" 



if __name__ == '__main__':

    # Create RKNN object
    rknn = RKNN(verbose=True)

    onnx_model = 'pp_humansegv2_mobile.onnx' 
    rknn_model = 'pp_human_segv2_custom_argmax.rknn'
    DATASET = './dataset.txt'
    QUANTIZE_ON = True

    rknn.config(mean_values=[127.5, 127.5, 127.5], std_values=[127.5, 127.5, 127.5], 
                quant_img_RGB2BGR=False,
                target_platform="rk3576")

    # customize argmax op
    ret = rknn.reg_custom_op(ArgMax)

    # Load ONNX model
    print('--> Loading model')
    ret = rknn.load_onnx(model=onnx_model, inputs=['x'], input_size_list=[[1,3,192,192]] )
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=QUANTIZE_ON, dataset=DATASET)
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')
 
    # Export RKNN model
    print('--> Export rknn model')
    ret = rknn.export_rknn(rknn_model)
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # init runtime environment
    print('--> Init runtime environment')
    ret = rknn.init_runtime()
    if ret != 0:
        print('Init runtime environment failed')
        exit(ret)
    print('done')

    img = cv.imread('../test_image.jpg')
    img_rgb = cv.cvtColor(img, cv.COLOR_BGR2RGB)
    img_rgb = cv.resize(img_rgb, (OUT_SIZE, OUT_SIZE)) 

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[img_rgb])
    np.save('./functions_custom_op_replace_onnx_0.npy', outputs[0])
    print('--> done')

    seg_mask = outputs[0].reshape(OUT_SIZE, OUT_SIZE) 
    vis_segmentation(img_rgb, seg_mask)

    rknn.release()
