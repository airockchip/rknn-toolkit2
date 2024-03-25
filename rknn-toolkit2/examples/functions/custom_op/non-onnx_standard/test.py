import numpy as np
from rknn.api import RKNN

OBJ_THRESH = 0.25
NMS_THRESH = 0.45
MODEL_IN_SIZE = (640, 640)

CLASSES = ("person", "bicycle", "car","motorbike ","aeroplane ","bus ","train","truck ","boat","traffic light",
           "fire hydrant","stop sign ","parking meter","bench","bird","cat","dog ","horse ","sheep","cow","elephant",
           "bear","zebra ","giraffe","backpack","umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball","kite",
           "baseball bat","baseball glove","skateboard","surfboard","tennis racket","bottle","wine glass","cup","fork","knife ",
           "spoon","bowl","banana","apple","sandwich","orange","broccoli","carrot","hot dog","pizza ","donut","cake","chair","sofa",
           "pottedplant","bed","diningtable","toilet ","tvmonitor","laptop	","mouse	","remote ","keyboard ","cell phone","microwave ",
           "oven ","toaster","sink","refrigerator ","book","clock","vase","scissors ","teddy bear ","hair drier", "toothbrush ")

coco_id_list = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 31, 32, 33, 34,
         35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
         64, 65, 67, 70, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 84, 85, 86, 87, 88, 89, 90]

class cstSigmoid:
    # custom operator cstSigmoid
    op_type = 'cstSigmoid'
    def shape_infer(self, node, in_shapes, in_dtypes):
        return in_shapes.copy(), in_dtypes.copy()
    def compute(self, node, inputs):
        return [1.0 / (1.0 + np.exp(np.negative(inputs[0])))]

class Letter_Box_Info():
    def __init__(self, shape, new_shape, w_ratio, h_ratio, dw, dh, pad_color) -> None:
        self.origin_shape = shape
        self.new_shape = new_shape
        self.w_ratio = w_ratio
        self.h_ratio = h_ratio
        self.dw = dw 
        self.dh = dh
        self.pad_color = pad_color

def filter_boxes(boxes, box_confidences, box_class_probs):
    """Filter boxes with object threshold.
    """
    box_confidences = box_confidences.reshape(-1)
    candidate, class_num = box_class_probs.shape

    class_max_score = np.max(box_class_probs, axis=-1)
    classes = np.argmax(box_class_probs, axis=-1)

    _class_pos = np.where(class_max_score* box_confidences >= OBJ_THRESH)
    scores = (class_max_score* box_confidences)[_class_pos]

    boxes = boxes[_class_pos]
    classes = classes[_class_pos]

    return boxes, classes, scores

def nms_boxes(boxes, scores):
    """Suppress non-maximal boxes.
    # Returns
        keep: ndarray, index of effective boxes.
    """
    x = boxes[:, 0]
    y = boxes[:, 1]
    w = boxes[:, 2] - boxes[:, 0]
    h = boxes[:, 3] - boxes[:, 1]

    areas = w * h
    order = scores.argsort()[::-1]

    keep = []
    while order.size > 0:
        i = order[0]
        keep.append(i)

        xx1 = np.maximum(x[i], x[order[1:]])
        yy1 = np.maximum(y[i], y[order[1:]])
        xx2 = np.minimum(x[i] + w[i], x[order[1:]] + w[order[1:]])
        yy2 = np.minimum(y[i] + h[i], y[order[1:]] + h[order[1:]])

        w1 = np.maximum(0.0, xx2 - xx1 + 0.00001)
        h1 = np.maximum(0.0, yy2 - yy1 + 0.00001)
        inter = w1 * h1

        ovr = inter / (areas[i] + areas[order[1:]] - inter)
        inds = np.where(ovr <= NMS_THRESH)[0]
        order = order[inds + 1]
    keep = np.array(keep)
    return keep

def box_process(position):
    grid_h, grid_w = position.shape[2:4]
    col, row = np.meshgrid(np.arange(0, grid_w), np.arange(0, grid_h))
    col = col.reshape(1, 1, grid_h, grid_w)
    row = row.reshape(1, 1, grid_h, grid_w)
    grid = np.concatenate((col, row), axis=1)
    stride = np.array([MODEL_IN_SIZE[1]//grid_h, MODEL_IN_SIZE[0]//grid_w]).reshape(1,2,1,1)

    box_xy = position[:,:2,:,:]
    box_wh = np.exp(position[:,2:4,:,:]) * stride

    box_xy += grid
    box_xy *= stride
    box = np.concatenate((box_xy, box_wh), axis=1)

    # Convert [c_x, c_y, w, h] to [x1, y1, x2, y2]
    xyxy = np.copy(box)
    xyxy[:, 0, :, :] = box[:, 0, :, :] - box[:, 2, :, :]/ 2  # top left x
    xyxy[:, 1, :, :] = box[:, 1, :, :] - box[:, 3, :, :]/ 2  # top left y
    xyxy[:, 2, :, :] = box[:, 0, :, :] + box[:, 2, :, :]/ 2  # bottom right x
    xyxy[:, 3, :, :] = box[:, 1, :, :] + box[:, 3, :, :]/ 2  # bottom right y

    return xyxy

def post_process(input_data):
    boxes, scores, classes_conf = [], [], []

    input_data = [_in.reshape([1, -1]+list(_in.shape[-2:])) for _in in input_data]
    for i in range(len(input_data)):
        boxes.append(box_process(input_data[i][:,:4,:,:]))
        scores.append(input_data[i][:,4:5,:,:])
        classes_conf.append(input_data[i][:,5:,:,:])

    def sp_flatten(_in):
        ch = _in.shape[1]
        _in = _in.transpose(0,2,3,1)
        return _in.reshape(-1, ch)

    boxes = [sp_flatten(_v) for _v in boxes]
    classes_conf = [sp_flatten(_v) for _v in classes_conf]
    scores = [sp_flatten(_v) for _v in scores]

    boxes = np.concatenate(boxes)
    classes_conf = np.concatenate(classes_conf)
    scores = np.concatenate(scores)

    # filter according to threshold
    boxes, classes, scores = filter_boxes(boxes, scores, classes_conf)

    # nms
    nboxes, nclasses, nscores = [], [], []
    keep = nms_boxes(boxes, scores)
    if len(keep) != 0:
        nboxes.append(boxes[keep])
        nclasses.append(classes[keep])
        nscores.append(scores[keep])

    if not nclasses and not nscores:
        return None, None, None

    boxes = np.concatenate(nboxes)
    classes = np.concatenate(nclasses)
    scores = np.concatenate(nscores)

    return boxes, classes, scores

def draw(image, boxes, scores, classes):
    for box, score, cl in zip(boxes, scores, classes):
        top, left, right, bottom = [int(_b) for _b in box]
        print('class: {}, score: {}'.format(CLASSES[cl], score))
        print('box coordinate left,top,right,down: [{}, {}, {}, {}]'.format(top, left, right, bottom))

        cv2.rectangle(image, (top, left), (right, bottom), (255, 0, 0), 2)
        cv2.putText(image, '{0} {1:.2f}'.format(CLASSES[cl], score),
                    (top, left - 6),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6, (0, 0, 255), 2)

def letter_box(im, new_shape, pad_color=(0,0,0)):
    # Resize and pad image while meeting stride-multiple constraints
    shape = im.shape[:2]  # current shape [height, width]
    if isinstance(new_shape, int):
        new_shape = (new_shape, new_shape)

    # Scale ratio
    h_ratio = new_shape[0] / shape[0]
    w_ratio = new_shape[1] / shape[1]
    r = min(h_ratio, w_ratio)

    # Compute padding
    new_unpad = int(round(shape[1] * r)), int(round(shape[0] * r))
    dw, dh = new_shape[1] - new_unpad[0], new_shape[0] - new_unpad[1]  # wh padding

    dw /= 2  # divide padding into 2 sides
    dh /= 2

    if shape[::-1] != new_unpad:  # resize
        im = cv2.resize(im, new_unpad, interpolation=cv2.INTER_LINEAR)
    top, bottom = int(round(dh - 0.1)), int(round(dh + 0.1))
    left, right = int(round(dw - 0.1)), int(round(dw + 0.1))
    im = cv2.copyMakeBorder(im, top, bottom, left, right, cv2.BORDER_CONSTANT, value=pad_color)  # add border

    letter_box_info = Letter_Box_Info(shape, new_shape, w_ratio, h_ratio, dw, dh, pad_color)        
    return im, letter_box_info

def get_real_box(box, letter_box_info, in_format='xyxy'):
    from copy import copy
    bbox = copy(box)
    # unletter_box result
    bbox[:,0] -= letter_box_info.dw
    bbox[:,0] /= letter_box_info.w_ratio
    bbox[:,0] = np.clip(bbox[:,0], 0, letter_box_info.origin_shape[1])

    bbox[:,1] -= letter_box_info.dh
    bbox[:,1] /= letter_box_info.h_ratio
    bbox[:,1] = np.clip(bbox[:,1], 0, letter_box_info.origin_shape[0])

    bbox[:,2] -= letter_box_info.dw
    bbox[:,2] /= letter_box_info.w_ratio
    bbox[:,2] = np.clip(bbox[:,2], 0, letter_box_info.origin_shape[1])

    bbox[:,3] -= letter_box_info.dh
    bbox[:,3] /= letter_box_info.h_ratio
    bbox[:,3] = np.clip(bbox[:,3], 0, letter_box_info.origin_shape[0])

    return bbox

def edit_onnx(in_model, output_model):
    # Here, take OP Sigmoid as an example. Users can replace any OP with their own custom OP in ONNX model.
    import onnx
    model = onnx.load(in_model)
    for node in model.graph.node:
        if node.op_type == "Sigmoid":
            node.op_type = "cstSigmoid"
    onnx.save(model, output_model)


if __name__ == '__main__':

    model_path = 'yolox_s.onnx'
    custom_model_path = 'yolox_s_custom.onnx'
    edit_onnx(model_path, custom_model_path)

    # Create RKNN object
    rknn = RKNN(verbose=True)

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[[0, 0, 0]], std_values=[[1, 1, 1]], target_platform='rk3588')
    print('done')

    # Register cstSigmoid op
    print('--> Register cstSigmoid op')
    ret = rknn.reg_custom_op(cstSigmoid())
    if ret != 0:
        print('Register cstSigmoid op failed!')
        exit(ret)
    print('done')

    # Load model
    print('--> Loading model')
    ret = rknn.load_onnx(model=custom_model_path, input_size_list=[[1, 3, MODEL_IN_SIZE[1], MODEL_IN_SIZE[0]]])
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=True, dataset='dataset.txt')
    if ret != 0:
        print('Build model failed!')
        exit(ret)
    print('done')

    # Export rknn model
    print('--> Export rknn model')
    ret = rknn.export_rknn('yolox_s_custom.rknn')
    if ret != 0:
        print('Export rknn model failed!')
        exit(ret)
    print('done')

    # Init runtime
    print('--> Init runtime')
    ret = rknn.init_runtime()
    if ret != 0:
        print('Init runtime failed!')
        exit(ret)
    print('done')

    # Get input data
    import cv2
    img_src = cv2.imread('bus.jpg')
    img, letter_box_info = letter_box(im=img_src.copy(), new_shape=(MODEL_IN_SIZE[1], MODEL_IN_SIZE[0]))
    print(letter_box_info)
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    # Simulator inference
    print('--> Inference model')
    outputs = rknn.inference(inputs=[img])
    np.save('./functions_custom_op_non-onnx_standard_0.npy', outputs[0])
    np.save('./functions_custom_op_non-onnx_standard_1.npy', outputs[1])
    np.save('./functions_custom_op_non-onnx_standard_2.npy', outputs[2])

    boxes, classes, scores = post_process(outputs)

    img_p = img_src.copy()
    if boxes is not None:
        draw(img_p, get_real_box(boxes, letter_box_info), scores, classes)
    cv2.imwrite('result.jpg', img_p)
    print('Save results to result.jpg!')

    # Release
    rknn.release()
