import torch
import torch.utils.cpp_extension
import numpy as np

cpp_source = """
#include <torch/script.h>
std::tuple<torch::Tensor, torch::Tensor> dual_residual(torch::Tensor x, torch::Tensor y, double_t alpha) {
    torch::Tensor tmp1 = x* alpha - y;
    torch::Tensor tmp2 = y* alpha - x;
    return std::tuple<torch::Tensor, torch::Tensor>(tmp1, tmp2);}
static auto registry = torch::RegisterOperators()
    .op("cst::dual_residual", &dual_residual);
"""

torch.utils.cpp_extension.load_inline(
    name="test",
    cpp_sources=cpp_source,
    functions="dual_residual",
    verbose=True,
    is_python_module=True,
)

class Model(torch.nn.Module):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.const = .5

    def forward(self, x, y):
        r1, r2 = torch.ops.cst.dual_residual(x, y, self.const)
        return r1, r2

x = torch.randn(1, 3, 10, 10)
y = torch.randn(1, 3, 10, 10)
m = Model()
z = m(x, y)

# target_result1 = x * .5 - y
# target_result2 = y * .5 - x
# print('result1 is equal:', torch.equal(z[0], target_result1))
# print('result2 is equal:', torch.equal(z[1], target_result2))

# jit_m = torch.jit.trace(m, (x, y))
# jit_m.save("dual_residual.pt")

from torch.onnx import register_custom_op_symbolic
from torch.onnx.symbolic_helper import parse_args

@parse_args('v', 'v', 'f')
def dual_residual_symbolic(g, x, y, alpha):
    output = g.op("rknn_cst::cstDualResidual", x, y, alpha_f=alpha, 
                outputs=2)
    return output

register_custom_op_symbolic("cst::dual_residual", dual_residual_symbolic, 12)
torch.onnx.export(m, (x, y), "dual_residual.onnx", opset_version=12)

np.save("dual_residual_input_0.npy", x.numpy())
np.save("dual_residual_input_1.npy", y.numpy())
np.save("dual_residual_output_0.npy", z[0].numpy())
np.save("dual_residual_output_1.npy", z[1].numpy())