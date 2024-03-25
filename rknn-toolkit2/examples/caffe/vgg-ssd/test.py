import os
import math
import numpy as np
import cv2
from rknn.api import RKNN

import PIL.Image as Image
import PIL.ImageDraw as ImageDraw
import PIL.ImageFont as ImageFont

np.set_printoptions(threshold=np.inf)

CLASSES = ('__background__',
           'aeroplane', 'bicycle', 'bird', 'boat',
           'bottle', 'bus', 'car', 'cat', 'chair',
           'cow', 'diningtable', 'dog', 'horse',
           'motorbike', 'person', 'pottedplant',
           'sheep', 'sofa', 'train', 'tvmonitor')

NUM_CLS = 21

CONF_THRESH = 0.5
NMS_THRESH = 0.45


def IntersectBBox(box1, box2):
    if box1[0] > box2[2] or box1[2] < box2[0] or box1[1] > box2[3] or box1[3] < box2[1]:
        return 0
    else:
        area1 = (box1[2] - box1[0]) * (box1[3] - box1[1])
        area2 = (box2[2] - box2[0]) * (box2[3] - box2[1])

        xx1 = max(box1[0], box2[0])
        yy1 = max(box1[1], box2[1])
        xx2 = min(box1[2], box2[2])
        yy2 = min(box1[3], box2[3])

        w = max(0, xx2-xx1)
        h = max(0, yy2-yy1)

        ovr = w*h / (area1 + area2 - w*h)
        return ovr


def ssd_post_process(conf_data, loc_data):
    prior_data = np.loadtxt('mbox_priorbox_97.txt', dtype=np.float32)

    prior_bboxes = prior_data[:len(loc_data)]
    prior_variances = prior_data[len(loc_data):]

    prior_num = int(len(loc_data) / 4)  # 8732

    conf_data = conf_data.reshape(-1, 21)

    idx_class_conf = []
    bboxes = []

    # conf
    for prior_idx in range(0, prior_num):
        max_val = np.max(conf_data[prior_idx])
        max_idx = np.argmax(conf_data[prior_idx])
        if max_val > CONF_THRESH and max_idx != 0:
            idx_class_conf.append([prior_idx, max_idx, max_val])

    # print(len(idx_class_conf))

    # boxes
    for i in range(0, prior_num):
        prior_w = prior_bboxes[4*i+2] - prior_bboxes[4*i]
        prior_h = prior_bboxes[4*i+3] - prior_bboxes[4*i+1]
        prior_center_x = (prior_bboxes[4*i+2] + prior_bboxes[4*i]) / 2
        prior_center_y = (prior_bboxes[4*i+3] + prior_bboxes[4*i+1]) / 2

        bbox_center_x = prior_variances[4*i+0] * loc_data[4*i+0][0] * prior_w + prior_center_x
        bbox_center_y = prior_variances[4*i+1] * loc_data[4*i+1][0] * prior_h + prior_center_y
        bbox_w = math.exp(prior_variances[4*i+2] * loc_data[4*i+2][0]) * prior_w
        bbox_h = math.exp(prior_variances[4*i+3] * loc_data[4*i+3][0]) * prior_h

        tmp = []
        tmp.append(max(min(bbox_center_x - bbox_w / 2., 1), 0))
        tmp.append(max(min(bbox_center_y - bbox_h / 2., 1), 0))
        tmp.append(max(min(bbox_center_x + bbox_w / 2., 1), 0))
        tmp.append(max(min(bbox_center_y + bbox_h / 2., 1), 0))
        bboxes.append(tmp)

    # print(len(idx_class_conf))

    # nms
    cur_class_num = 0
    idx_class_conf_ = []
    for i in range(0, len(idx_class_conf)):
        keep = True
        k = 0
        while k < cur_class_num:
            if keep:
                ovr = IntersectBBox(bboxes[idx_class_conf[i][0]], bboxes[idx_class_conf_[k][0]])
                if idx_class_conf_[k][1] == idx_class_conf[i][1] and ovr > NMS_THRESH:
                    if idx_class_conf_[k][2] < idx_class_conf[i][2]:
                        idx_class_conf_.pop(k)
                        idx_class_conf_.append(idx_class_conf[i])
                    keep = False
                    break
                k += 1
            else:
                break
        if keep:
            idx_class_conf_.append(idx_class_conf[i])
            cur_class_num += 1

    # print(idx_class_conf_)

    box_class_score = []

    for i in range(0, len(idx_class_conf_)):
        bboxes[idx_class_conf_[i][0]].append(idx_class_conf_[i][1])
        bboxes[idx_class_conf_[i][0]].append(idx_class_conf_[i][2])
        box_class_score.append(bboxes[idx_class_conf_[i][0]])

    img = cv2.imread('./road_300x300.jpg')
    img_pil = Image.fromarray(img)
    draw = ImageDraw.Draw(img_pil)

    font = ImageFont.load_default()

    if len(box_class_score) != 0:
        print("{:^12} {:^12}  {}".format('class', 'score', 'xmin, ymin, xmax, ymax'))
        print('-' * 50)

    for i in range(0, len(box_class_score)):
        x1 = int(box_class_score[i][0]*img.shape[1])
        y1 = int(box_class_score[i][1]*img.shape[0])
        x2 = int(box_class_score[i][2]*img.shape[1])
        y2 = int(box_class_score[i][3]*img.shape[0])
        color = (0, int(box_class_score[i][4]/20.0*255), 255)
        draw.line([(x1, y1), (x1, y2), (x2, y2),
                   (x2, y1), (x1, y1)], width=2, fill=color)
        display_str = CLASSES[box_class_score[i][4]] + ":" + str(box_class_score[i][5])
        try:
            display_str_height = np.ceil((1 + 2 * 0.05) * font.getbbox(display_str)[3])+1
        except:
            display_str_height = np.ceil((1 + 2 * 0.05) * font.getsize(display_str)[1])+1

        if y1 > display_str_height:
            text_bottom = y1
        else:
            text_bottom = y1 + display_str_height

        try:
            _, _, text_width, text_height = font.getbbox(display_str)
        except:
            text_width, text_height = font.getsize(display_str)
        margin = np.ceil(0.05 * text_height)
        draw.rectangle([(x1, text_bottom-text_height-2*margin), (x1+text_width, text_bottom)], fill=color)
        draw.text((x1+margin, text_bottom-text_height-margin), display_str, fill='black', font=font)

        print("{:^12} {:^12.3f} [{:>4}, {:>4}, {:>4}, {:>4}]".format(CLASSES[box_class_score[i][4]], box_class_score[i][5],
                                                                     x1, y1, x2, y2))

    np.copyto(img, np.array(img_pil))
    cv2.imwrite("result.jpg", img)
    print('Save results to result.jpg!')


if __name__ == '__main__':

    if not os.path.exists('./VGG_VOC0712_SSD_300x300_iter_120000.caffemodel'):
        print('!!! Missing VGG_VOC0712_SSD_300x300_iter_120000.caffemodel !!!\n'
              '1. Download models_VGGNet_VOC0712_SSD_300x300.tar.gz from https://drive.google.com/file/d/0BzKzrI_SkD1_WVVTSmQxU0dVRzA/view\n'
              '2. Extract the VGG_VOC0712_SSD_300x300_iter_120000.caffemodel from models_VGGNet_VOC0712_SSD_300x300.tar.gz\n'
              '3. Or you can also download caffemodel from https://ftzr.zbox.filez.com/v2/delivery/data/95f00b0fc900458ba134f8b180b3f7a1/asset/vgg-ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel\n')
        exit(-1)

    # Create RKNN object
    rknn = RKNN(verbose=True)

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[103.94, 116.78, 123.68], std_values=[1, 1, 1], quant_img_RGB2BGR=True, target_platform='rk3566')
    print('done')

    # Load model
    print('--> Loading model')
    ret = rknn.load_caffe(model='./deploy_rm_detection_output.prototxt',
                          blobs='./VGG_VOC0712_SSD_300x300_iter_120000.caffemodel')
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

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn('./deploy_rm_detection_output.rknn')
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # Set inputs
    img = cv2.imread('./road_300x300.jpg')
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
    print('done')

    outputs[0] = outputs[0].reshape((-1, 1))
    outputs[1] = outputs[1].reshape((-1, 1))
    np.save('./caffe_vgg-ssd_0.npy', outputs[0])
    np.save('./caffe_vgg-ssd_1.npy', outputs[1])
    ssd_post_process(outputs[1], outputs[0])

    rknn.release()
