from rknn.api import RKNN

# Test on the original onnx model
rknn = RKNN(verbose=True)
rknn.config(target_platform='rk3588')
rknn.load_onnx("./concat_block.onnx")
rknn.build(do_quantization=True, dataset='./dataset.txt')
rknn.export_rknn('./concat_block.rknn')

# use rknn.utils.onnx_edit to edit the onnx model
from rknn.utils import onnx_edit
ret = onnx_edit(model = './concat_block.onnx', 
                export_path = './concat_block_edited.onnx', 
                inputs_transform = { 'k_cache.1': 'a,b,c,d->1,ad,b,c'},
                outputs_transform = {'k_cache': 'a,b,c,d->1,ab,c,d'},
                dataset = './dataset.txt'
                )

# Test on the edited onnx model
rknn = RKNN(verbose=True)
rknn.config(target_platform='rk3588')
rknn.load_onnx("./concat_block_edited.onnx")
rknn.build(do_quantization=True, dataset='./dataset_for_concat_block_edited.txt')
rknn.export_rknn('./concat_block_edited.rknn')
