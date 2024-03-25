import cv2
import numpy as np
import platform
from synset_label import labels
from rknnlite.api import RKNNLite

# decice tree for RK356x/RK3576/RK3588
DEVICE_COMPATIBLE_NODE = '/proc/device-tree/compatible'

def get_host():
    # get platform and device type
    system = platform.system()
    machine = platform.machine()
    os_machine = system + '-' + machine
    if os_machine == 'Linux-aarch64':
        try:
            with open(DEVICE_COMPATIBLE_NODE) as f:
                device_compatible_str = f.read()
                if 'rk3588' in device_compatible_str:
                    host = 'RK3588'
                elif 'rk3562' in device_compatible_str:
                    host = 'RK3562'
                elif 'rk3576' in device_compatible_str:
                    host = 'RK3576'
                else:
                    host = 'RK3566_RK3568'
        except IOError:
            print('Read device node {} failed.'.format(DEVICE_COMPATIBLE_NODE))
            exit(-1)
    else:
        host = os_machine
    return host

INPUT_SIZE = 224

RK3566_RK3568_RKNN_MODEL = 'mobilenet_v2_for_rk3566_rk3568.rknn'
RK3588_RKNN_MODEL = 'mobilenet_v2_for_rk3588.rknn'
RK3562_RKNN_MODEL = 'mobilenet_v2_for_rk3562.rknn'
RK3576_RKNN_MODEL = 'mobilenet_v2_for_rk3576.rknn'


def show_top5(result):
    output = result[0].reshape(-1)
    # Get the indices of the top 5 largest values
    output_sorted_indices = np.argsort(output)[::-1][:5]
    top5_str = '-----TOP 5-----\n'
    for i, index in enumerate(output_sorted_indices):
        value = output[index]
        if value > 0:
            topi = '[{:>3d}] score:{:.6f} class:"{}"\n'.format(
                index, value, labels[index])
        else:
            topi = '-1: 0.0\n'
        top5_str += topi
    print(top5_str)


if __name__ == '__main__':

    # Get device information
    host_name = get_host()
    if host_name == 'RK3566_RK3568':
        rknn_model = RK3566_RK3568_RKNN_MODEL
    elif host_name == 'RK3562':
        rknn_model = RK3562_RKNN_MODEL
    elif host_name == 'RK3576':
        rknn_model = RK3576_RKNN_MODEL
    elif host_name == 'RK3588':
        rknn_model = RK3588_RKNN_MODEL
    else:
        print("This demo cannot run on the current platform: {}".format(host_name))
        exit(-1)

    dynamic_input = [
        [[1, 3, 192, 192]],
        [[1, 3, 256, 256]],
        [[1, 3, 160, 160]],
        [[1, 3, 224, 224]]
    ]

    rknn_lite = RKNNLite()

    # Load RKNN model
    print('--> Load RKNN model')
    ret = rknn_lite.load_rknn(rknn_model)
    if ret != 0:
        print('Load RKNN model failed')
        exit(ret)
    print('done')

    img = cv2.imread('./dog_224x224.jpg')

    # Init runtime environment
    print('--> Init runtime environment')
    # Run on RK356x / RK3576 / RK3588 with Debian OS, do not need specify target.
    if host_name in ['RK3576', 'RK3588']:
        # For RK3576 / RK3588, specify which NPU core the model runs on through the core_mask parameter.
        ret = rknn_lite.init_runtime(core_mask=RKNNLite.NPU_CORE_0)
    else:
        ret = rknn_lite.init_runtime()
    if ret != 0:
        print('Init runtime environment failed')
        exit(ret)
    print('done')

    # Inference
    print('--> Running model')
    print('model: mobilenet_v2\n')
    print('input shape: 1,3,224,224')
    real_img = cv2.resize(img, (224, 224))
    real_img = np.expand_dims(real_img, 0)
    real_img = np.transpose(real_img, (0, 3, 1, 2))
    outputs = rknn_lite.inference(inputs=[real_img], data_format=['nchw'])
    # Show the classification results
    show_top5(outputs)

    print('input shape: 1,3,160,160')
    real_img = cv2.resize(img, (160, 160))
    real_img = np.expand_dims(real_img, 0)
    real_img = np.transpose(real_img, (0, 3, 1, 2))
    outputs = rknn_lite.inference(inputs=[real_img], data_format=['nchw'])
    # Show the classification results
    show_top5(outputs)

    print('input shape: 1,3,256,256')
    real_img = cv2.resize(img, (256, 256))
    real_img = np.expand_dims(real_img, 0)
    real_img = np.transpose(real_img, (0, 3, 1, 2))
    outputs = rknn_lite.inference(inputs=[real_img], data_format=['nchw'])
    # Show the classification results
    show_top5(outputs)

    print('done')

    rknn_lite.release()
