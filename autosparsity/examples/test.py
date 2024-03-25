import numpy as np
import cv2
from rknn.api import RKNN


ONNX_MODEL = 'resnet50.onnx'
RKNN_MODEL = 'resnet50.rknn'

def show_outputs(outputs):
    output = outputs[0][0]
    index = sorted(range(len(output)), key=lambda k : output[k], reverse=True)
    fp = open('./labels.txt', 'r')
    labels = fp.readlines()
    top5_str = 'resnet50_sparse_infer\n-----TOP 5-----\n'
    for i in range(5):
        value = output[index[i]]
        if value > 0:
            topi = '[{:>3d}] score:{:.6f} class:"{}"\n'.format(index[i], value, labels[index[i]].strip().split(':')[-1])
        else:
            topi = '[ -1]: 0.0\n'
        top5_str += topi
    print(top5_str.strip())


if __name__ == '__main__':

    # Create RKNN object
    rknn = RKNN(verbose=True)

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[123.675, 116.28, 103.53], std_values=[58.395, 57.12, 57.375], target_platform='rk3576', sparse_infer=True)
    print('done')

    # Load model
    print('--> Loading model')
    ret = rknn.load_onnx(model=ONNX_MODEL)
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset='./datasets.txt')
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn(RKNN_MODEL)
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
    ret = rknn.init_runtime(target='rk3576')
    if ret != 0:
        print('Init runtime environment failed!')
        exit(ret)
    print('done')

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[img], data_format=['nhwc'])
    x = outputs[0]
    output = np.exp(x)/np.sum(np.exp(x))
    outputs = [output]
    show_outputs(outputs)
    print('done')

    rknn.release()
