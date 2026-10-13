#encoding=utf-8
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import tensorflow as tf
import numpy as np
from numpy import random as nr
import cv2

if int(tf.__version__.split('.')[0]) >= 2:
    tf = tf.compat.v1
    tf.disable_v2_behavior()

tf.logging.set_verbosity(tf.logging.INFO)



outputShape={1:[8,8,3]}
    



def newtork(x1, x2, x3, x4):
    #weights = tf.get_variable("w1",shape=[3,3,3,1],initializer=tf.truncated_normal_initializer(stddev=1.0))
    weights1 = tf.Variable(tf.ones([3, 3, 3, 1]))
    conv1 = tf.nn.conv2d(x1,weights1,strides=[1, 1, 1, 1],padding='SAME')

    weights2 = tf.Variable(tf.ones([3, 3, 3, 1]))
    conv2 = tf.nn.conv2d(x2, weights2, strides=[1, 1, 1, 1], padding='SAME')

    weights3 = tf.Variable(tf.ones([3, 3, 3, 1]))
    conv3 = tf.nn.conv2d(x3, weights3, strides=[1, 1, 1, 1], padding='SAME')

    weights4 = tf.Variable(tf.ones([3, 3, 1, 1]))
    conv4 = tf.nn.conv2d(x4, weights4, strides=[1, 1, 1, 1], padding='SAME')

    add2 = tf.add(conv1, conv2)
    add3 = tf.add(add2, conv3)
    add4 = tf.add(add3, conv4, name="output")
    return add4
    
    
def train():
    inWidth = 128 
    inHeight = 128 
    inChannel = 3
    
    img = cv2.imread('./dog_128x128.jpg')
    img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    img = img.astype(np.float32)
    img[:,:,:] -= [127.5, 127.5, 127.5]
    img[:,:,:] /= 128.0
    img = img.reshape((1,128,128,3))

    img_gray = cv2.imread('./dog_128x128_gray.png', cv2.IMREAD_GRAYSCALE)
    img_gray = img_gray.astype(np.float32)
    img_gray[:, :] -= [127.5]
    img_gray[:, :] /= 128.0
    img_gray = img_gray.reshape((1, 128, 128, 1))
    
    x1 = tf.placeholder(tf.float32,[1,inHeight,inWidth,inChannel],name="input1")
    my_input1 = img

    x2 = tf.placeholder(tf.float32, [1, inHeight, inWidth, 3], name="input2")
    #my_input2 = np.ones([1, inHeight, inWidth, 5],dtype=np.float32)
    #my_input2 = np.zeros([1, inHeight, inWidth, 5], dtype=np.float32)
    my_input2 = nr.rand(1, inHeight, inWidth, 3)  # np.ones([1, inHeight, inWidth, 5],dtype=np.float32)
    my_input2 = (my_input2 - 0.5) * 2
    # RKNN calibration and inference expect NCHW; TensorFlow feeds use NHWC.
    np.save('input2.npy', my_input2.transpose(0, 3, 1, 2))

    x3 = tf.placeholder(tf.float32, [1, inHeight, inWidth, 3], name="input3")
    #my_input3 = np.ones([1, inHeight, inWidth, 4], dtype=np.float32)
    #my_input3 = np.zeros([1, inHeight, inWidth, 4], dtype=np.float32)
    my_input3 = nr.rand(1, inHeight, inWidth, 3)
    my_input3 = (my_input3 - 0.5) * 2
    np.save('input3.npy', my_input3.transpose(0, 3, 1, 2))
    
    x4 = tf.placeholder(tf.float32,[1,inHeight,inWidth,1],name="input4")
    my_input4 = img_gray

    layerNum = 1
    outShape=outputShape[layerNum]
    y = tf.placeholder(tf.float32,[1,outShape[2],outShape[1],outShape[0]])
    my_label = np.ones([1,outShape[2],outShape[1],outShape[0]],dtype=np.float32)
    

    predict_res = newtork(x1, x2, x3, x4)

    #set loss func
    # loss = tf.reduce_mean(predict_res-y)
    # train_step = tf.train.AdamOptimizer(0.0005).minimize(loss)

    #iterate for 10
    pb_name='conv_128.pb'
    with tf.Session() as sess:
        sess.run(tf.global_variables_initializer())
        print(sess.run(predict_res,feed_dict={x1:my_input1, x2:my_input2, x3:my_input3, x4:my_input4, y:my_label}))

        print(predict_res)
        print([n.name for n in tf.get_default_graph().as_graph_def().node])
        
        constant_graph = tf.graph_util.convert_variables_to_constants(sess, sess.graph_def, output_node_names=['output'])
        with tf.gfile.GFile(pb_name, mode='wb') as f:
            f.write(constant_graph.SerializeToString())
            
        tf.train.Saver().save(sess,"model.ckpt")

def main():
    train()

if __name__=="__main__":
    main()
        
