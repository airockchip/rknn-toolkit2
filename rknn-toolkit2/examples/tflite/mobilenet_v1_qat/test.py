import numpy as np
import cv2
from rknn.api import RKNN


def show_outputs(outputs):
    output = outputs[0][0]
    index = sorted(range(len(output)), key=lambda k : output[k], reverse=True)
    fp = open('./labels.txt', 'r')
    labels = fp.readlines()
    top5_str = 'mobilenet_v1\n-----TOP 5-----\n'
    for i in range(5):
        value = output[index[i]]
        if value > 0:
            topi = '[{:>4d}] score:{:.6f} class:"{}"\n'.format(index[i], value, labels[index[i]].strip().split(':')[-1])
        else:
            topi = '[  -1]: 0.0\n'
        top5_str += topi
    print(top5_str.strip())

def dequantize(outputs, scale, zp):
    outputs[0] = (outputs[0] - zp) * scale
    return outputs

if __name__ == '__main__':

    # Create RKNN object
    rknn = RKNN(verbose=True)

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[128, 128, 128], std_values=[128, 128, 128], target_platform='rk3566')
    print('done')

    # Load model (from https://www.tensorflow.org/lite/examples/image_classification/overview?hl=zh-cn)
    print('--> Loading model')
    ret = rknn.load_tflite(model='mobilenet_v1_1.0_224_quant.tflite')
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn('./mobilenet_v1.rknn')
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # Set inputs
    img = cv2.imread('./dog_224x224.jpg')
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img = np.expand_dims(img, 0)

    # Init runtime environment
    print('--> Init runtime environment')
    ret = rknn.init_runtime()
    if ret != 0:
        print('Init runtime environment failed!')
        exit(ret)
    print('done')

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[img], data_format=['nhwc'])
    np.save('./tflite_mobilenet_v1_qat_0.npy', outputs[0])
    show_outputs(dequantize(outputs, scale=0.00390625, zp=0))
    print('done')

    rknn.release()
