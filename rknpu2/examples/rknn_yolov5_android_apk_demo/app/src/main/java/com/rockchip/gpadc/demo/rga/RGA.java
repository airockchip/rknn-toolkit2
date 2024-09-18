package com.rockchip.gpadc.demo.rga;

import android.util.Log;

public class RGA {
    static {
        System.loadLibrary("rknn4j");
    }

    /**
     * Convert color format and flip image if flip is set true.
     * Please refer to the link to confirm the RGA driver version, make sure it is higher than 1.2.4
     * https://github.com/airockchip/librga/blob/main/docs/Rockchip_FAQ_RGA_CN.md#rga-driver
     * @param src: image raw data, which need be processed.
     * @param srcFmt: source color format, use the color format defined in HALDefine.
     * @param dst: destination memory address.
     * @param dstFmt: destination color format, use the color format defined in HALDefine.
     * @param width: image width.
     * @param height: image height.
     * @param flip: flip type, use the flip defined in HALDefine, if not flip, set -1.
     * @return: -1 means failure,
     */
    public static int colorConvertAndFlip(byte[] src, int srcFmt, long npu_buf_handle, int dstFmt,
                                   int width, int height, int flip) {

        if (src == null) {
            Log.w("rkyolo.RGA", "src or dst is null");
            return -1;
        }

        return color_convert_and_flip(src, srcFmt, npu_buf_handle, dstFmt, width, height, flip);
    }

    private static native int color_convert_and_flip(byte[] src, int srcFmt, long npu_buf_handle, int dstFmt,
                                            int width, int height, int flip);
}
