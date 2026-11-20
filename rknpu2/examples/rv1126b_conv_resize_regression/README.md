# RV1126B regression: 3×3 Conv followed by 4× nearest Resize

A deterministic, synthetic FP16 test. No model weights, images or dataset are required. It uses ONNX opset 12 and `optimization_level=3`, with quantization disabled. The graph input is `[1,1,72,128]`; the convolution produces nine channels with one sample per 3×3 neighborhood tap. Both two-op graphs must return `[1,9,288,512]`.

## Reproduce

1. On the conversion host, in the RKNN-Toolkit2 environment with `numpy` and `onnx` installed:

   ```sh
   python -I repro.py generated
   ```

2. Build `run.c` with the RKNN C SDK and an AArch64 cross-compiler (or use `cc` on the board if available). Point `RKNN_API` to `rknpu2/runtime/Linux/librknn_api`:

   ```sh
   aarch64-linux-gnu-gcc -std=c99 -O2 -Wall -Wextra -I"$RKNN_API/include" run.c -L"$RKNN_API/aarch64" -lrknnrt -o run
   ```

3. Copy `run` and `generated` to the RV1126B board. Run on the device, using the runtime you want to test. You can use a directory with a matching AArch64 `librknnrt.so` on `LD_LIBRARY_PATH` to select a different runtime for this test without replacing the system copy. The runner prints the actual runtime and driver versions. Inputs are float32 NHWC; outputs are float32 NCHW.

   ```sh
   mkdir -p actual
   for name in conv_only resize_only conv_resize resize_dilated_conv; do
     ./run "generated/$name.rknn" "generated/${name}_input.bin" "actual/${name}_0.bin" || exit 2
   done
   ```

4. Copy `actual` back to the conversion host and compare against the NumPy reference. Exit status 1 means at least one output was wrong.

   ```sh
   python -I repro.py generated --compare actual
   ```

## Expected vs. observed

With Toolkit2 **2.3.2**, runtime **2.3.2** (`429f97ae6b`), RKNPU driver **0.9.8**, and RV1126B AArch64 hardware:

| Graph | Maximum absolute error | Elements with error > 0.01 |
|---|---:|---:|
| 3×3 Conv alone | 0 | 0 / 82,944 |
| 4× nearest Resize alone | 0 | 0 / 1,327,104 |
| 3×3 Conv → 4× nearest Resize | 31.75 | 1,299,456 / 1,327,104 |
| 4× nearest Resize → 3×3 Conv, dilation/padding 4 | 0 | 0 / 1,327,104 |

Running the **same Toolkit2 2.3.2 artifacts** with runtime **2.5.0** (`74ea630ce8`) via `LD_LIBRARY_PATH` produced identical comparison metrics. **Rebuilding with Toolkit2 2.5.0** and running with runtime **2.5.0** on the same board and driver made all four cases pass with zero error, including the original Conv → Resize graph. Upgrade the converter and regenerate the `.rknn` files; replacing only the runtime cannot correct these older artifacts.

The fact that both operators pass separately points to the combined path. This test does not determine whether the root cause is the converter, runtime, or their interaction. It is distinct from [the bilinear Resize → Conv issue](https://github.com/airockchip/rknn-toolkit2/issues/579).

## Graph-level workaround

For this stride-one, zero-padded convolution followed by integer nearest-neighbor upsampling (`asymmetric` / `floor`), move the resize before the convolution and multiply the convolution's dilation and padding by the upscale factor. The weights are unchanged. With a factor of four, the index identity `floor((i + 4k) / 4) = floor(i / 4) + k` preserves both the tap locations and the zero-padded boundary. The comparison above verifies the replacement on hardware. This is not a patch to `librknnrt.so` or a general rewrite for other interpolation modes, strides or coordinate transforms.
