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
        tmp_2 = y* alpha - x
        return [tmp_1, tmp_2] 



if __name__ == '__main__':

    custom_model_path = 'dual_residual.onnx'

    # Create RKNN object
    rknn = RKNN()

    # Pre-process config
    print('--> Config model')
    rknn.config(mean_values=[[0, 0, 0]], std_values=[[1, 1, 1]], target_platform='rk3566')
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
    ret = rknn.load_onnx(model=custom_model_path, input_size_list=[[1, 3, 10, 10]])
    if ret != 0:
        print('Load model failed!')
        exit(ret)
    print('done')

    # Build model
    print('--> Building model')
    ret = rknn.build(do_quantization=False, dataset=None)
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

    # Release
    rknn.release()
