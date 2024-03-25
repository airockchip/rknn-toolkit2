import numpy as np
from rknn.api import RKNN


def show_outputs(outputs):
    np.save('./caffe_mobilenet_v2_0.npy', outputs[0])
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
    rknn = RKNN(verbose=False)

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[103.94, 116.78, 123.68], std_values=[58.82, 58.82, 58.82], quant_img_RGB2BGR=True, target_platform='rk3562')
    print('done')

    # Load model
    print('--> Loading model')
    ret = rknn.load_caffe(model='../../caffe/mobilenet_v2/mobilenet_v2_deploy.prototxt',
                          blobs='../../caffe/mobilenet_v2/mobilenet_v2.caffemodel')
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset='../../caffe/mobilenet_v2/dataset.txt')
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn('./mobilenet_v2.rknn')
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    print('--> Generate cpp demo')
    ret = rknn.codegen(output_path='./rknn_app_demo', inputs=['../../caffe/mobilenet_v2/dog_224x224.jpg'], overwrite=True)
    if ret != 0:
        print('Generate cpp demo failed!')
        exit(ret)
    print('done')

    rknn.release()
