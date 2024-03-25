import numpy as np
from rknn.api import RKNN
from rknn.api.custom_op import get_node_attr
from scipy.spatial import distance


class cstDualResidual:
    # custom operator cstDualResidual 
    op_type = 'cstDualResidual'
    def shape_infer(self, node, in_shapes, in_dtypes):
        return in_shapes.copy(), in_dtypes.copy()
    def compute(self, node, inputs):
        x = inputs[0]
        y = inputs[1]
        alpha = get_node_attr(node, 'alpha')
        tmp_1 = x*alpha - y  
        tmp_2 = y*alpha - x
        return [tmp_1, tmp_2] 


if __name__ == '__main__':

    custom_model_path = 'dual_residual.onnx'

    # Create RKNN object
    rknn = RKNN(verbose=True)

    # Pre-process config
    print('--> Config model')
    rknn.config(target_platform='rk3588')
    print('done')

    # Register cstSigmoid op
    print('--> Register cstDualResidual op')
    ret = rknn.reg_custom_op(cstDualResidual())
    if ret != 0:
        print('Register cstDualResidual op failed!')
        exit(ret)
    print('done')

    # Load model
    print('--> Loading model')
    ret = rknn.load_onnx(model=custom_model_path)
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
    ret = rknn.export_rknn('dual_residual_custom.rknn')
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

    in_data_0 = np.load('dual_residual_input_0.npy') 
    in_data_1 = np.load('dual_residual_input_1.npy') 

    in_data_0 = np.transpose(in_data_0, (0,2,3,1))
    in_data_1 = np.transpose(in_data_1, (0,2,3,1))
    
    outputs = rknn.inference(inputs=[in_data_0, in_data_1])

    # verifying
    golden_data_0 = np.load('dual_residual_output_0.npy').astype(np.float32).ravel() 
    golden_data_1 = np.load('dual_residual_output_1.npy').astype(np.float32).ravel() 

    cosine_dst = 1 - distance.cosine(outputs[0].astype(np.float32).ravel(), golden_data_0)
    print(f"cos distance for 0th output: {cosine_dst:.5f}")
    cosine_dst = 1 - distance.cosine(outputs[1].astype(np.float32).ravel(), golden_data_1)
    print(f"cos distance for 1th output: {cosine_dst:.5f}")

    # Release
    rknn.release()
