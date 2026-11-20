// SPDX-License-Identifier: Apache-2.0
// Run a single-input/single-output model: float32 NHWC in, float32 NCHW out.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rknn_api.h"

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s MODEL.rknn INPUT.bin OUTPUT.bin\n", argv[0]);
        return 2;
    }

    rknn_context ctx;
    int rc = rknn_init(&ctx, argv[1], 0, 0, NULL);
    if (rc) {
        fprintf(stderr, "rknn_init: %d\n", rc);
        return 2;
    }
    const char *stage = "query SDK version";
    rknn_sdk_version ver = {0};
    rknn_input_output_num count = {0};
    rknn_tensor_attr attr = {0}, out_attr = {0};
    rknn_input in = {0};
    rknn_output out = {0};
    FILE *f = NULL;
    int done = 0, have_output = 0;
    if ((rc = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &ver, sizeof(ver))))
        goto cleanup;
    printf("Runtime: %s; driver: %s\n", ver.api_version, ver.drv_version);

    stage = "query tensor count";
    if ((rc = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &count, sizeof(count))) ||
        count.n_input != 1 || count.n_output != 1)
        goto cleanup;
    stage = "query tensor attributes";
    if ((rc = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &attr, sizeof(attr))) ||
        (rc = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &out_attr, sizeof(out_attr))))
        goto cleanup;
    if (!attr.n_elems || attr.n_elems > UINT32_MAX / sizeof(float) ||
        !out_attr.n_elems || out_attr.n_elems > UINT32_MAX / sizeof(float) ||
        out_attr.fmt != RKNN_TENSOR_NCHW)
        goto cleanup;

    stage = "read input";
    in.size = attr.n_elems * sizeof(float);
    in.buf = malloc(in.size);
    in.type = RKNN_TENSOR_FLOAT32;
    in.fmt = RKNN_TENSOR_NHWC;
    f = fopen(argv[2], "rb");
    if (!f || !in.buf || fread(in.buf, 1, in.size, f) != in.size ||
        fgetc(f) != EOF || ferror(f))
        goto cleanup;
    fclose(f);
    f = NULL;

    stage = "run model";
    out.want_float = 1;
    if ((rc = rknn_inputs_set(ctx, 1, &in)) || (rc = rknn_run(ctx, NULL)) ||
        (rc = rknn_outputs_get(ctx, 1, &out, NULL)))
        goto cleanup;
    have_output = 1;

    stage = "write output";
    if (out.size != out_attr.n_elems * sizeof(float))
        goto cleanup;
    f = fopen(argv[3], "wb");
    if (!f || fwrite(out.buf, 1, out.size, f) != out.size || fflush(f))
        goto cleanup;
    rc = fclose(f);
    f = NULL;
    done = rc == 0;

cleanup:
    if (!done)
        fprintf(stderr, "%s failed (status %d)\n", stage, rc);
    if (f)
        fclose(f);
    if (have_output)
        rknn_outputs_release(ctx, 1, &out);
    free(in.buf);
    rknn_destroy(ctx);
    return done ? 0 : 2;
}
