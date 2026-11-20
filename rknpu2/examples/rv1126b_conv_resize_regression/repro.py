#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Generate FP16 RKNN regression cases for Conv -> nearest Resize on RV1126B."""
import argparse
from pathlib import Path

import numpy as np

H, W, SCALE = 72, 128, 4
CASES = ('conv_only', 'resize_only', 'conv_resize', 'resize_dilated_conv')


def build(out):
    from onnx import TensorProto, checker, helper as h, numpy_helper as nh, save
    from rknn.api import RKNN

    out.mkdir(parents=True, exist_ok=True)
    # Binary fractions that are exact in FP16. Nonperiodic input exposes stride errors.
    x = np.random.default_rng(541).integers(-256, 257, (1, 1, H, W)).astype(np.float32) / 16
    kernels = np.eye(9, dtype=np.float32).reshape(9, 1, 3, 3)
    p = np.pad(x, ((0, 0), (0, 0), (1, 1), (1, 1)))
    neighbours = np.concatenate([p[:, :, i:i+H, j:j+W] for i in range(3) for j in range(3)], axis=1)
    expected = neighbours.repeat(SCALE, axis=2).repeat(SCALE, axis=3)

    def conv(src, dst, dilation=1):
        return h.make_node('Conv', [src, 'kernel'], [dst],
                           pads=[dilation]*4, dilations=[dilation]*2, kernel_shape=[3, 3])

    def resize(src, dst):
        return h.make_node('Resize', [src, 'roi', 'scales'], [dst], mode='nearest',
                           coordinate_transformation_mode='asymmetric', nearest_mode='floor')

    k = nh.from_array(kernels, 'kernel')
    r = [nh.from_array(np.empty(0, dtype=np.float32), 'roi'),
         nh.from_array(np.array([1, 1, SCALE, SCALE], dtype=np.float32), 'scales')]
    specs = (
        ([conv('x', 'y')], [k], x, neighbours),
        ([resize('x', 'y')], r, neighbours, expected),
        ([conv('x', 'mid'), resize('mid', 'y')], [k, *r], x, expected),
        ([resize('x', 'mid'), conv('mid', 'y', SCALE)], [k, *r], x, expected),
    )
    for name, (nodes, initializers, src, want) in zip(CASES, specs):
        graph = h.make_graph(nodes, name,
                             [h.make_tensor_value_info('x', TensorProto.FLOAT, src.shape)],
                             [h.make_tensor_value_info('y', TensorProto.FLOAT, want.shape)], initializers)
        model = h.make_model(graph, opset_imports=[h.make_opsetid('', 12)], ir_version=8)
        checker.check_model(model)
        path = out / f'{name}.onnx'
        save(model, path)
        src.transpose(0, 2, 3, 1).astype('<f4').tofile(out / f'{name}_input.bin')
        want.astype('<f4').tofile(out / f'{name}_expected.bin')
        rknn = RKNN()
        try:
            for step, kwargs in [('config', dict(target_platform='rv1126b', optimization_level=3)),
                                 ('load_onnx', dict(model=str(path))),
                                 ('build', dict(do_quantization=False)),
                                 ('export_rknn', dict(export_path=str(out / f'{name}.rknn')))]:
                if getattr(rknn, step)(**kwargs) != 0:
                    raise RuntimeError(f'{name}: {step} failed')
        finally:
            rknn.release()


def compare(out, actual):
    passed = True
    for name in CASES:
        got = np.fromfile(actual / f'{name}_0.bin', dtype='<f4')
        want = np.fromfile(out / f'{name}_expected.bin', dtype='<f4')
        if got.shape != want.shape or not np.isfinite(got).all():
            print(f'{name}: FAIL (size mismatch or non-finite output)')
            passed = False
            continue
        delta = np.abs(got - want)
        good = bool(delta.max() <= 0.01)
        passed &= good
        print(f'{name}: {"PASS" if good else "FAIL"}, max abs {delta.max():.6g}, '
              f'mean abs {delta.mean():.6g}, mismatches {(delta > 0.01).sum()}/{delta.size}')
    return 0 if passed else 1


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('directory', type=Path, help='directory to generate models, inputs and expected outputs')
    p.add_argument('--compare', type=Path, metavar='ACTUAL', help='compare board float32 outputs instead of building')
    args = p.parse_args()
    if args.compare:
        raise SystemExit(compare(args.directory, args.compare))
    build(args.directory)
