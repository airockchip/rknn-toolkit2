import numpy as np
import cv2
from rknn.api import RKNN


def show_outputs(outputs):
    output = outputs[0].reshape(-1)
    index = sorted(range(len(output)), key=lambda k : output[k], reverse=True)
    fp = open('./labels.txt', 'r')
    labels = fp.readlines()
    top5_str = 'mobilenet_v2\n-----TOP 5-----\n'
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
    rknn.config(mean_values=[103.94, 116.78, 123.68], std_values=[58.82, 58.82, 58.82],
                quant_img_RGB2BGR=True, target_platform='rk3588')
    print('done')

    # Load model (from https://github.com/shicai/MobileNet-Caffe)
    print('--> Loading model')
    ret = rknn.load_caffe(model='./../../caffe/mobilenet_v2/mobilenet_v2_deploy.prototxt',
                          blobs='./../../caffe/mobilenet_v2/mobilenet_v2.caffemodel')
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset='./dataset.txt')
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')

    # Accuracy analysis
    print('--> Accuracy analysis')
    ret = rknn.accuracy_analysis(inputs=['./dog_224x224.jpg'], output_dir='./snapshot', target='rk3588')
    if ret != 0:
        print('Accuracy analysis failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn('./mobilenet_v2.rknn')
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # Export encrypted RKNN model
    print('--> Export encrypted rknn model')
    ret = rknn.export_encrypted_rknn_model('./mobilenet_v2.rknn', None, 3)

    rknn.release()


    # Create RKNN object
    rknn = RKNN(verbose=True)

    # load rknn model
    print('--> Load rknn model')
    ret = rknn.load_rknn('./mobilenet_v2.rknn')
    if ret != 0:
        print('Load rknn model failed!')
        exit(ret)
    print('done')

    # Set inputs
    img = cv2.imread('./dog_224x224.jpg')
    img = np.expand_dims(img, 0)

    print('--> List devices')
    rknn.list_devices()

    # Init runtime environment
    print('--> Init runtime environment')
    ret = rknn.init_runtime(target='rk3588', perf_debug=True, eval_mem=True)
    if ret != 0:
        print('Init runtime environment failed!')
        exit(ret)
    print('done')

    print('--> Get sdk version')
    sdk_version = rknn.get_sdk_version()
    print(sdk_version)

    # eval perf
    print('--> Eval perf')
    rknn.eval_perf()

    # eval perf
    print('--> Eval memory')
    rknn.eval_memory()

    # Inference
    print('--> Running model')
    outputs = rknn.inference(inputs=[img], data_format=['nhwc'])
    np.save('./functions_npu_device_test_0.npy', outputs[0])
    show_outputs(outputs)
    print('done')

    rknn.release()
