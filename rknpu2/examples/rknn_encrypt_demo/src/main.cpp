/****************************************************************************
 *
 *    Copyright (c) 2026 by Rockchip Corp.  All rights reserved.
 *
 *    Demo: load an encrypted RKNN model and run inference in zero-copy mode.
 *
 *    By default the encrypted model is loaded into a buffer and passed to
 *    rknn_init with its size. Define LOAD_FROM_PATH at compile time to pass
 *    the model file path directly (size = 0) instead.
 *
 *    Input/output tensors use zero-copy memory from rknn_create_mem() and
 *    rknn_set_io_mem(), which is also compatible with the RV1106/RV1103
 *    family mini runtime (librknnmrt). When CMake builds for
 *    RV1106_RV1103/RV1106B_RV1103B it defines RV1106_RV1103/RV1106B_RV1103B,
 *    which excludes rknn_set_core_mask() (single NPU core, not provided by
 *    the mini runtime).
 *
 *    Usage: ./rknn_encrypt_demo <enc_model.rknn> <key_envelope.bin>
 *                              [input_path] [loop_count] [core_mask]
 *
 *****************************************************************************/

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "rknn_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define LOAD_FROM_PATH

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

/*-------------------------------------------
                  Functions
-------------------------------------------*/
static inline int64_t getCurrentTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

static int rknn_GetTopN(float* pfProb, float* pfMaxProb, uint32_t* pMaxClass, uint32_t outputCount, uint32_t topNum)
{
  uint32_t i, j;
  uint32_t top_count = outputCount > topNum ? topNum : outputCount;

  for (i = 0; i < topNum; ++i) {
    pfMaxProb[i] = -1.0f;
    pMaxClass[i] = (uint32_t)-1;
  }

  for (j = 0; j < top_count; j++) {
    for (i = 0; i < outputCount; i++) {
      uint32_t k;
      int      skip = 0;
      for (k = 0; k < topNum; k++) {
        if (i == pMaxClass[k]) {
          skip = 1;
          break;
        }
      }
      if (skip) continue;
      if (pfProb[i] > pfMaxProb[j]) {
        pfMaxProb[j]  = pfProb[i];
        pMaxClass[j] = i;
      }
    }
  }
  return 0;
}

static unsigned char* load_file(const char* path, size_t* size)
{
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    printf("open %s failed!\n", path);
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  long sz = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  unsigned char* buf = (unsigned char*)malloc(sz);
  if (!buf) {
    fclose(fp);
    return NULL;
  }
  size_t n = fread(buf, 1, sz, fp);
  fclose(fp);
  if ((long)n != sz) {
    free(buf);
    return NULL;
  }
  *size = (size_t)sz;
  return buf;
}

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
  printf("  index=%d, name=%s, n_dims=%d, n_elems=%d, size=%d, fmt=%d, type=%d, qnt_type=%d, zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, attr->n_elems, attr->size, attr->fmt, attr->type, attr->qnt_type,
         attr->zp, attr->scale);
}

/*-------------------------------------------
                  Main
-------------------------------------------*/
int main(int argc, char* argv[])
{
  if (argc < 3) {
    printf("Usage:%s <enc_model.rknn> <key_envelope.bin> [input_path] [loop_count] [core_mask]\n", argv[0]);
    return -1;
  }

  const char* enc_model_path = argv[1];
  const char* key_env_path   = argv[2];
  const char* input_path     = argc > 3 ? argv[3] : NULL;
  int         loop_count     = argc > 4 ? atoi(argv[4]) : 1;
  uint32_t    core_mask      = argc > 5 ? strtoul(argv[5], NULL, 10) : 1;

  /* 1. Load the key envelope (always needed for encrypted models). */
  size_t key_env_size = 0;
  void*  key_env_data = load_file(key_env_path, &key_env_size);
  if (!key_env_data) {
    printf("load key envelope %s failed!\n", key_env_path);
    return -1;
  }
  printf("Key envelope:    %s (%zu bytes)\n", key_env_path, key_env_size);

  rknn_init_extend      init_ext;
  rknn_decrypt_key_mem  key_mem;
  memset(&init_ext, 0, sizeof(rknn_init_extend));
  key_mem.key_data = key_env_data;
  key_mem.key_size = (uint32_t)key_env_size;
  init_ext.user_key = &key_mem;

  rknn_context ctx = 0;
  uint32_t     flag = 0;

  int64_t t0 = getCurrentTimeUs();
#ifdef LOAD_FROM_PATH
  /* Init rknn from encrypted model file path (size = 0 signals file path).
   * The runtime opens the file, detects RKCE magic, and decrypts internally. */
  printf("init rknn from model path: %s\n", enc_model_path);
  int ret = rknn_init(&ctx, (void*)enc_model_path, 0, flag, &init_ext);
#else
  /* Init rknn from encrypted model buffer. */
  size_t enc_model_size = 0;
  void*  enc_model_data = load_file(enc_model_path, &enc_model_size);
  if (!enc_model_data) {
    printf("load encrypted model %s failed!\n", enc_model_path);
    free(key_env_data);
    return -1;
  }
  printf("Encrypted model: %s (%zu bytes)\n", enc_model_path, enc_model_size);
  int ret = rknn_init(&ctx, enc_model_data, (uint32_t)enc_model_size, flag, &init_ext);
#endif
  int64_t t1 = getCurrentTimeUs();
  printf("rknn_init (with decryption) ret=%d, took %lld us\n", ret, (long long)(t1 - t0));
  if (ret < 0) {
    printf("rknn_init fail! ret=%d\n", ret);
    if (ret == RKNN_ERR_KEY_INVALID) {
      printf("  -> RKNN_ERR_KEY_INVALID: key envelope invalid / RSA unwrap failed.\n");
    } else if (ret == RKNN_ERR_DECRYPT_FAILED) {
      printf("  -> RKNN_ERR_DECRYPT_FAILED: AES-CTR model body decryption failed.\n");
    } else if (ret == RKNN_ERR_SOC_BIND_FAILED) {
      printf("  -> RKNN_ERR_SOC_BIND_FAILED: current SoC not in the bound platform list.\n");
    }
#ifndef LOAD_FROM_PATH
    free(enc_model_data);
#endif
    free(key_env_data);
    return -1;
  }

#ifndef LOAD_FROM_PATH
  free(enc_model_data);
#endif
  memset(key_env_data, 0, key_env_size);
  free(key_env_data);

#if defined(RV1106_RV1103) || defined(RV1106B_RV1103B)
  (void)core_mask;
  printf("RV1106/RV1103 family: single NPU core, rknn_set_core_mask skipped\n");
#else
  ret = rknn_set_core_mask(ctx, (rknn_core_mask)core_mask);
  if (ret != RKNN_SUCC) {
    printf("rknn_set_core_mask fail! ret=%d (ignored on single-core platforms)\n", ret);
  }
#endif

  rknn_sdk_version sdk_ver;
  ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &sdk_ver, sizeof(sdk_ver));
  if (ret != RKNN_SUCC) {
    printf("rknn_query SDK_VERSION fail! ret=%d\n", ret);
  } else {
    printf("rknn_api/rknnrt version: %s, driver version: %s\n", sdk_ver.api_version, sdk_ver.drv_version);
  }

  rknn_input_output_num io_num;
  ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (ret != RKNN_SUCC) {
    printf("rknn_query IN_OUT_NUM fail! ret=%d\n", ret);
    rknn_destroy(ctx);
    return -1;
  }
  printf("model inputs: %d, outputs: %d\n", io_num.n_input, io_num.n_output);

  rknn_tensor_attr input_attrs[io_num.n_input];
  memset(input_attrs, 0, sizeof(input_attrs));
  for (uint32_t i = 0; i < io_num.n_input; i++) {
    input_attrs[i].index = i;
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &input_attrs[i], sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC) {
      printf("rknn_query INPUT_ATTR[%u] fail! ret=%d\n", i, ret);
      rknn_destroy(ctx);
      return -1;
    }
    dump_tensor_attr(&input_attrs[i]);
  }

  /* 3. Query output attributes in native NHWC layout, as needed for the
   *    zero-copy output memory below. */
  rknn_tensor_attr output_attrs[io_num.n_output];
  memset(output_attrs, 0, sizeof(output_attrs));
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    output_attrs[i].index = i;
    ret = rknn_query(ctx, RKNN_QUERY_NATIVE_NHWC_OUTPUT_ATTR, &output_attrs[i], sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC) {
      printf("rknn_query NATIVE_NHWC_OUTPUT_ATTR[%u] fail! ret=%d\n", i, ret);
      rknn_destroy(ctx);
      return -1;
    }
    dump_tensor_attr(&output_attrs[i]);
  }

  /* Zero-copy tensor memory (declared before any error goto below). */
  rknn_tensor_mem* input_mems[io_num.n_input];
  uint8_t*         input_data[io_num.n_input];
  rknn_tensor_mem* output_mems[io_num.n_output];
  int64_t          total_us = 0;
  for (uint32_t i = 0; i < io_num.n_input; i++) {
    input_mems[i] = NULL;
    input_data[i] = NULL;
  }
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    output_mems[i] = NULL;
  }

  for (uint32_t i = 0; i < io_num.n_input; i++) {
    input_attrs[i].type = RKNN_TENSOR_UINT8;
    input_attrs[i].fmt  = RKNN_TENSOR_NHWC;

    int req_h = input_attrs[i].dims[1];
    int req_w = input_attrs[i].dims[2];
    int req_c = input_attrs[i].dims[3];

    /* packed NHWC input data (no stride) */
    input_data[i] = (uint8_t*)malloc((size_t)req_w * req_h * req_c);
    if (!input_data[i]) {
      printf("malloc input[%u] data failed!\n", i);
      goto err;
    }
    memset(input_data[i], 0, (size_t)req_w * req_h * req_c);

    if (input_path) {
      int img_c = 0, img_w = 0, img_h = 0;
      unsigned char* img = stbi_load(input_path, &img_w, &img_h, &img_c, req_c);
      if (img) {
        /* naive nearest resize into the input buffer (req_w x req_h x req_c) */
        for (int y = 0; y < req_h; y++) {
          int sy = img_h * y / req_h;
          for (int x = 0; x < req_w; x++) {
            int sx = img_w * x / req_w;
            memcpy(input_data[i] + (size_t)(y * req_w + x) * req_c, img + (size_t)(sy * img_w + sx) * req_c, req_c);
          }
        }
        stbi_image_free(img);
      } else {
        printf("load input image %s failed; using zeros\n", input_path);
      }
    }

    input_mems[i] = rknn_create_mem(ctx, input_attrs[i].size_with_stride);
    if (!input_mems[i]) {
      printf("rknn_create_mem(input[%u]) failed!\n", i);
      goto err;
    }

    /* copy the packed data into tensor memory, honoring the width stride */
    {
      uint32_t stride = input_attrs[i].w_stride;
      if (stride == 0 || (int)stride == req_w) {
        memcpy(input_mems[i]->virt_addr, input_data[i], (size_t)req_w * req_h * req_c);
      } else {
        uint8_t* src     = input_data[i];
        uint8_t* dst     = (uint8_t*)input_mems[i]->virt_addr;
        size_t   src_row = (size_t)req_w * req_c;
        size_t   dst_row = (size_t)stride * req_c;
        for (int y = 0; y < req_h; y++) {
          memcpy(dst, src, src_row);
          src += src_row;
          dst += dst_row;
        }
      }
    }

    ret = rknn_set_io_mem(ctx, input_mems[i], &input_attrs[i]);
    if (ret != RKNN_SUCC) {
      printf("rknn_set_io_mem(input[%u]) fail! ret=%d\n", i, ret);
      goto err;
    }
  }

  /* 5. Zero-copy outputs: float tensor memory sized for the dequantized
   *    native NHWC output. */
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    output_attrs[i].type = RKNN_TENSOR_FLOAT32;
    output_mems[i]       = rknn_create_mem(ctx, output_attrs[i].n_elems * sizeof(float));
    if (!output_mems[i]) {
      printf("rknn_create_mem(output[%u]) failed!\n", i);
      goto err;
    }
    ret = rknn_set_io_mem(ctx, output_mems[i], &output_attrs[i]);
    if (ret != RKNN_SUCC) {
      printf("rknn_set_io_mem(output[%u]) fail! ret=%d\n", i, ret);
      goto err;
    }
  }

  /* 6. Run inference loop. Outputs are read directly from the zero-copy
   *    tensor memory, so only rknn_run is needed per iteration. */
  for (int loop = 0; loop < loop_count; loop++) {
    t0 = getCurrentTimeUs();
    ret = rknn_run(ctx, NULL);
    t1 = getCurrentTimeUs();
    total_us += (t1 - t0);
    if (ret != RKNN_SUCC) {
      printf("rknn_run fail! ret=%d\n", ret);
      goto err;
    }

    /* Top-5 of the first output (classification-style). */
    if (io_num.n_output > 0) {
      uint32_t out_elems = output_attrs[0].n_elems;
      float    max_prob[5];
      uint32_t max_cls[5];
      rknn_GetTopN((float*)output_mems[0]->virt_addr, max_prob, max_cls, out_elems, 5);
      printf("--- loop %d top5 ---\n", loop);
      for (int k = 0; k < 5; k++) {
        printf("  #%d: class=%u, prob=%.6f\n", k, max_cls[k], max_prob[k]);
      }
    }
  }

  printf("avg rknn_run time over %d loops: %.3f ms\n", loop_count, total_us / 1000.0f / loop_count);

  /* 7. Cleanup. */
  for (uint32_t i = 0; i < io_num.n_input; i++) {
    if (input_mems[i]) rknn_destroy_mem(ctx, input_mems[i]);
    free(input_data[i]);
  }
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    if (output_mems[i]) rknn_destroy_mem(ctx, output_mems[i]);
  }
  rknn_destroy(ctx);

  printf("done.\n");
  return 0;

err:
  for (uint32_t i = 0; i < io_num.n_input; i++) {
    if (input_mems[i]) rknn_destroy_mem(ctx, input_mems[i]);
    free(input_data[i]);
  }
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    if (output_mems[i]) rknn_destroy_mem(ctx, output_mems[i]);
  }
  rknn_destroy(ctx);
  return -1;
}
