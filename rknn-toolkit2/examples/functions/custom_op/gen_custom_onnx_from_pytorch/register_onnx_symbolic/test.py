import torch
import numpy as np

torch.manual_seed(0)

class Model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.m = torch.nn.Threshold(0.1, 20)

    def forward(self, x):
        y = self.m(x)
        return y

from torch.onnx import register_custom_op_symbolic
from torch.onnx.symbolic_helper import parse_args

@parse_args('v', 'f', 'f')
def threshold_symbolic(g, x, threshold, value):
    # output = g.op("rknn_cst::cst_threshold", x)                                             # ignore threshold and value definition
    output = g.op("rknn_cst::cstThreshold", x, threshold_f=threshold, value_f=value)       # define threshold and value individually
    # output = g.op("rknn_cst::cst_threshold", x, param_f=[threshold,value])                  # combine threshold and value into a list
    return output
register_custom_op_symbolic("aten::threshold", threshold_symbolic, 12)

x = torch.randn(1, 3, 10, 10)
m = Model()
result = m(x)
torch.onnx.export(m, (x,), "threshold.onnx", opset_version=12)

np.save("threshold_input.npy", x.numpy())
np.save("threshold_output.npy", result.numpy())
