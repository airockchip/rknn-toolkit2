/****************************************************************************
 *
 *    Copyright (c) 2017 - 2024 by Rockchip Corp.  All rights reserved.
 *
 *    The material in this file is confidential and contains trade secrets
 *    of Rockchip Corporation. This is proprietary information owned by
 *    Rockchip Corporation. No part of this work may be disclosed,
 *    reproduced, copied, transmitted, or used in any way for any purpose,
 *    without the express written permission of Rockchip Corporation.
 *
 *****************************************************************************/

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "fp16/Float16.h"
#include "matmul_utils.h"
#include "rknn_api.h"
#include "rknn_matmul_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <algorithm>
#include <string>
#include <vector>
using namespace rknpu2;

/*-------------------------------------------
                  Functions
-------------------------------------------*/
static inline int64_t getCurrentTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void set_mem_from_int8_to_int4(int8_t *dst, int8_t *src, int size)
{
  for (int idx = 0; idx < size; idx += 2)
  {
    dst[idx / 2] = ((src[idx]) & 0xf) | ((src[idx + 1] << 4) & 0xf0);
  }
}

// 一维矩阵乘法函数
template <typename Ti, typename To>
std::vector<To> matrixMultiply(const Ti *A, const Ti *B, int M, int K, int N)
{
  std::vector<To> result(M * N, 0);

  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      float sum = 0;
      for (int k = 0; k < K; ++k)
      {
        sum += (float)A[i * K + k] * (float)B[k * N + j];
      }
      result[i * N + j] = sum;
    }
  }

  return result;
}

// 一维矩阵混合量化乘法函数
template <typename Ta, typename Tb, typename Tc>
std::vector<Tc> matrixMultiplyWithHybridQuant(const Ta *A, const Tb *B, int M, int K, int N, float input_scale,
                                              float weight_scale, float output_scale)
{
  std::vector<Tc> result(M * N, 0);

  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      float sum = 0;
      for (int k = 0; k < K; ++k)
      {
        sum += (float)A[i * K + k] * (float)B[k * N + j];
      }
      result[i * N + j] = sum * input_scale * weight_scale / output_scale;
    }
  }

  return result;
}

// 一维矩阵乘法函数
std::vector<int8_t> matrixMultiplyWithQuant(const int8_t *A, const int8_t *B, int M, int K, int N, float input_scale,
                                            float weight_scale, float output_scale)
{
  std::vector<int8_t> result(M * N, 0);

  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      float sum = 0;
      for (int k = 0; k < K; ++k)
      {
        sum += (float)A[i * K + k] * (float)B[k * N + j];
      }
      result[i * N + j] = sum * input_scale * weight_scale / output_scale;
    }
  }

  return result;
}

static const char *get_dims_string(rknn_matmul_tensor_attr *attr)
{
  if (!attr->n_dims)
  {
    return "()";
  }
  static char dims_str[128];
  memset(&dims_str[0], 0, sizeof(dims_str));
  sprintf(&dims_str[0], "(%d", attr->dims[0]);
  for (uint32_t i = 1; i < attr->n_dims; ++i)
  {
    int idx = strlen(dims_str);
    sprintf(&dims_str[idx], ", %d", attr->dims[i]);
  }
  strcat(&dims_str[0], ")");
  return dims_str;
}

static void dump_matmul_tensor_attr(rknn_matmul_tensor_attr *attr)
{
  printf("  name=%s, dims=%s, size=%d, type=%s\n", attr->name, get_dims_string(attr), attr->size,
         get_type_string(attr->type));
}

static int8_t get_virt_addr_int4(void *virt_addr, int index)
{
  int8_t int4 = 0;
  if (index % 2 == 0)
  {
    int4 = (((int8_t *)virt_addr)[index / 2] >> 4) & 0xf;
  }
  else
  {
    int4 = (((int8_t *)virt_addr)[index / 2]) & 0xf;
  }
  if (int4 & 0x8)
  {
    int4 = int4 | 0xf0;
  }
  return int4;
}

static void dump_matmul_tensor(rknn_tensor_mem *tensor, rknn_matmul_tensor_attr *attr)
{
  printf("  %s%s:\n", attr->name, get_dims_string(attr));
  // normal layout
  if (attr->n_dims == 2)
  {
    for (uint32_t i = 0; i < attr->dims[0]; ++i)
    {
      for (uint32_t j = 0; j < attr->dims[1]; ++j)
      {
        void *virt_addr = (void *)((size_t)tensor->virt_addr + tensor->offset);
        if (attr->type == RKNN_TENSOR_INT8)
        {
          printf(" %4d", ((int8_t *)virt_addr)[i * attr->dims[1] + j]);
        }
        else if (attr->type == RKNN_TENSOR_INT32)
        {
          printf(" %6d", ((int32_t *)virt_addr)[i * attr->dims[1] + j]);
        }
        else if (attr->type == RKNN_TENSOR_FLOAT16)
        {
          printf(" %5.2f", (float)(((float16 *)virt_addr)[i * attr->dims[1] + j]));
        }
        else if (attr->type == RKNN_TENSOR_FLOAT32)
        {
          printf(" %5.2f", ((float *)virt_addr)[i * attr->dims[1] + j]);
        }
        else if (attr->type == RKNN_TENSOR_INT16)
        {
          printf(" %d", ((int16_t *)virt_addr)[i * attr->dims[1] + j]);
        }
        else if (attr->type == RKNN_TENSOR_INT4)
        {
          int index = i * attr->dims[1] + j;
          int8_t int4 = get_virt_addr_int4(virt_addr, index);
          printf("%d ", int4);
        }
      }
      printf("\n");
    }
    printf("\n");
  }
  // perf layout
  else if (attr->n_dims == 3)
  {
    for (uint32_t i = 0; i < attr->dims[0]; ++i)
    {
      for (uint32_t j = 0; j < attr->dims[1]; ++j)
      {
        for (uint32_t k = 0; k < attr->dims[2]; ++k)
        {
          void *virt_addr = (void *)((size_t)tensor->virt_addr + tensor->offset);
          if (attr->type == RKNN_TENSOR_INT4)
          {
            int index = (i * attr->dims[1] + j) * attr->dims[2] + k;
            int8_t int4 = get_virt_addr_int4(virt_addr, index);
            printf("%d ", int4);
          }
          else if (attr->type == RKNN_TENSOR_INT8)
          {
            printf(" %4d ", ((int8_t *)virt_addr)[(i * attr->dims[1] + j) * attr->dims[2] + k]);
          }
          else if (attr->type == RKNN_TENSOR_INT16)
          {
            printf(" %6d ", ((int16_t *)virt_addr)[(i * attr->dims[1] + j) * attr->dims[2] + k]);
          }
          else if (attr->type == RKNN_TENSOR_INT32)
          {
            printf(" %6d ", ((int32_t *)virt_addr)[(i * attr->dims[1] + j) * attr->dims[2] + k]);
          }
          else if (attr->type == RKNN_TENSOR_FLOAT16)
          {
            printf(" %5.2f ", (float)(((float16 *)virt_addr)[(i * attr->dims[1] + j) * attr->dims[2] + k]));
          }
          else if (attr->type == RKNN_TENSOR_FLOAT32)
          {
            printf(" %5.2f ", ((float *)virt_addr)[(i * attr->dims[1] + j) * attr->dims[2] + k]);
          }
        }
        printf("\n");
      }
      printf("\n");
    }
  }
  // native layout
  else if (attr->n_dims == 4)
  {
    // N / 16
    for (uint32_t n = 0; n < attr->dims[0]; ++n)
    {
      // K / 32
      for (uint32_t k = 0; k < attr->dims[1]; ++k)
      {
        // 16
        for (uint32_t nn = 0; nn < attr->dims[2]; ++nn)
        {
          // 32
          for (uint32_t kk = 0; kk < attr->dims[3]; kk++)
          {
            void *virt_addr = (void *)((size_t)tensor->virt_addr + tensor->offset);
            if (attr->type == RKNN_TENSOR_INT4)
            {
              int index = ((n * attr->dims[1] + k) * attr->dims[2] + nn) * attr->dims[3] + kk;
              int8_t int4 = get_virt_addr_int4(virt_addr, index);
              printf("%d ", int4);
            }
            else if (attr->type == RKNN_TENSOR_INT8)
            {
              printf(" %4d ",
                     ((int8_t *)virt_addr)[((n * attr->dims[1] + k) * attr->dims[2] + nn) * attr->dims[3] + kk]);
            }
            else if (attr->type == RKNN_TENSOR_INT32)
            {
              printf(" %6d ",
                     ((int32_t *)virt_addr)[((n * attr->dims[1] + k) * attr->dims[2] + nn) * attr->dims[3] + kk]);
            }
            else if (attr->type == RKNN_TENSOR_FLOAT16)
            {
              printf(
                  " %5.2f ",
                  (float)(((float16 *)virt_addr)[((n * attr->dims[1] + k) * attr->dims[2] + nn) * attr->dims[3] + kk]));
            }
            else if (attr->type == RKNN_TENSOR_FLOAT32)
            {
              printf(" %5.2f ",
                     ((float *)virt_addr)[((n * attr->dims[1] + k) * attr->dims[2] + nn) * attr->dims[3] + kk]);
            }
          }
          printf("\n");
        }
        printf("\n");
      }
      printf("\n");
    }
  }
}

static std::vector<std::string> split(const std::string &str, const std::string &pattern)
{
  std::vector<std::string> res;
  if (str == "")
    return res;
  std::string strs = str + pattern;
  size_t pos = strs.find(pattern);
  while (pos != strs.npos)
  {
    std::string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos = strs.find(pattern);
  }
  return res;
}

static void print_usage(char *argv[])
{
  printf("Usage:\n%s <matmul_type> <M1K1N1#M2K2N2#...> <B_layout> <AC_layout> <loop_count> <core_mask>\n", argv[0]);
  printf("\tM_shapes:         M shape array, which separeted by ',' \n");
  printf("\tmatmul_type = 1: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32\n");
  printf("\tmatmul_type = 2: RKNN_INT8_MM_INT8_TO_INT32\n");
  printf("\tmatmul_type = 4: RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16\n");
  printf("\tmatmul_type = 7: RKNN_FLOAT16_MM_INT4_TO_FLOAT32\n");
  printf("\tmatmul_type = 10: RKNN_INT4_MM_INT4_TO_INT16\n");
  printf("Example: A = [1,64]#[4,64]#[8,64], B = [64,32], int8 matmul test command as followed:\n");
  printf("  feature+const: %s 2 1,64,32#4,64,32#8,64,32 1 1\n", argv[0]);
  printf("  two feature:   %s 2 1,64,32#4,64,32#8,64,32 2 1\n", argv[0]);
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    print_usage(argv);
    return -1;
  }
  int loop_count = 10;
  int print_tensor = 1;
  rknn_matmul_type matmul_type = (rknn_matmul_type)atoi(argv[1]);
  // check matmul_type value range
  if (matmul_type < 1 || matmul_type > 11)
  {
    fprintf(stderr, "invalid matmul_type: %d, required matmul_type =1~11!\n", matmul_type);
    print_usage(argv);
    return -1;
  }

  // Parse matmul_shapes, as "M1,K1,N1#M2,K2,N2#..."
  std::vector<rknn_matmul_shape> shapes;
  std::vector<std::string> MKNs = split(argv[2], "#");

  shapes.resize(MKNs.size());
  for (int i = 0; i < MKNs.size(); ++i)
  {
    std::vector<std::string> MKN_strs = split(MKNs[i], ",");
    if (MKN_strs.size() != 3)
    {
      fprintf(stderr, "MKN splited by # must be 3 number!\n");
      print_usage(argv);
      return -1;
    }
    shapes[i].M = std::atoi(MKN_strs[0].c_str());
    shapes[i].K = std::atoi(MKN_strs[1].c_str());
    shapes[i].N = std::atoi(MKN_strs[2].c_str());
  }
  const int shape_num = shapes.size();
  int maxM = std::max_element(shapes.begin(), shapes.end(), [](const rknn_matmul_shape &a, const rknn_matmul_shape &b)
                              { return a.M < b.M; })
                 ->M;
  int maxK = std::max_element(shapes.begin(), shapes.end(), [](const rknn_matmul_shape &a, const rknn_matmul_shape &b)
                              { return a.K < b.K; })
                 ->K;
  int maxN = std::max_element(shapes.begin(), shapes.end(), [](const rknn_matmul_shape &a, const rknn_matmul_shape &b)
                              { return a.N < b.N; })
                 ->N;

  // request normal or native layout for B
  int B_layout = 0;
  if (argc > 3)
  {
    B_layout = atoi(argv[3]);
  }
  if (B_layout != 2)
  {
    // Check B_layout = 0 or 1 when all dynamic_shapes. K and N must be the same
    for (int i = 0; i < shape_num; ++i)
    {
      if (shapes[i].K != shapes[0].K || shapes[i].N != shapes[0].N)
      {
        fprintf(stderr, "B_layout = 0 or 1, all shapes.K and N must be same!\n");
        print_usage(argv);
        return -1;
      }
    }
  }

  // request normal or perf layout for A and C
  int AC_layout = 0;
  if (argc > 4)
  {
    AC_layout = atoi(argv[4]);
  }

  if (argc > 5)
  {
    loop_count = atoi(argv[5]);
  }

  int core_mask = 0;
  if (argc > 6)
  {
    core_mask = atoi(argv[6]);
  }

  if (argc > 7)
  {
    print_tensor = atoi(argv[7]);
  }

  printf("MatMul matmul_type = %s, MKNs = %s, B_layout = %d, AC_layout = %d, loop_count = %d, core_mask = %d\n",
         get_matmul_type_string(matmul_type), argv[2], B_layout, AC_layout, loop_count, core_mask);

  rknn_matmul_ctx ctx;

  rknn_matmul_info info;
  memset(&info, 0, sizeof(rknn_matmul_info));
  info.M = maxM; // malloc data by max M shape
  info.K = maxK;
  info.N = maxN;
  info.type = matmul_type;
  info.B_layout = B_layout;
  info.AC_layout = AC_layout;

  if (matmul_type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32 || matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 ||
      matmul_type == RKNN_INT8_MM_INT4_TO_INT32)
  {
    info.B_quant_type = RKNN_QUANT_TYPE_PER_CHANNEL_SYM;
  }

  if (matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 || matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 ||
      matmul_type == RKNN_INT8_MM_INT8_TO_FLOAT32)
  {
    info.B_quant_type = RKNN_QUANT_TYPE_PER_GROUP_SYM;
    info.group_size = 32;
  }

  rknn_matmul_io_attr io_attr[shape_num];
  memset(io_attr, 0, sizeof(rknn_matmul_io_attr) * shape_num);

  int ret = rknn_matmul_create_dynamic_shape(&ctx, &info, shape_num, shapes.data(), io_attr);
  if (ret < 0)
  {
    fprintf(stderr, "rknn_matmul_create fail! ret=%d\n", ret);
    return -1;
  }

  ret = rknn_matmul_set_core_mask(ctx, (rknn_core_mask)core_mask);

  printf("support input/output attribute:\n");
  for (int i = 0; i < shape_num; i++)
  {
    printf("------------------------------- shape[%d] -------------------------------\n", i);
    dump_matmul_tensor_attr(&io_attr[i].A);
    dump_matmul_tensor_attr(&io_attr[i].B);
    dump_matmul_tensor_attr(&io_attr[i].C);
  }
  printf("------------------------------------------------------------------------\n");

  // set A/B/C quant params
  if (matmul_type == RKNN_INT8_MM_INT8_TO_INT8)
  {
    rknn_quant_params params_a;
    memcpy(params_a.name, io_attr[0].A.name, RKNN_MAX_NAME_LEN);
    params_a.scale_len = 1;
    params_a.scale = (float *)malloc(params_a.scale_len * sizeof(float));
    params_a.scale[0] = 0.2;
    params_a.zp_len = 1;
    params_a.zp = (int32_t *)malloc(params_a.zp_len * sizeof(int32_t));
    params_a.zp[0] = 0;
    rknn_matmul_set_quant_params(ctx, &params_a);

    rknn_quant_params params_b;
    memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
    params_b.scale_len = 1;
    params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
    params_b.scale[0] = 0.1;
    params_b.zp_len = 1;
    params_b.zp = (int32_t *)malloc(params_b.zp_len * sizeof(int32_t));
    params_b.zp[0] = 0;
    rknn_matmul_set_quant_params(ctx, &params_b);

    rknn_quant_params params_c;
    memcpy(params_c.name, io_attr[0].C.name, RKNN_MAX_NAME_LEN);
    params_c.scale_len = 1;
    params_c.scale = (float *)malloc(params_c.scale_len * sizeof(float));
    params_c.scale[0] = 0.8;
    params_c.zp_len = 1;
    params_c.zp = (int32_t *)malloc(params_c.zp_len * sizeof(int32_t));
    params_c.zp[0] = 0;
    rknn_matmul_set_quant_params(ctx, &params_c);
  }

  if (matmul_type == RKNN_INT8_MM_INT4_TO_INT32)
  {
    rknn_quant_params params_a;
    memcpy(params_a.name, io_attr[0].A.name, RKNN_MAX_NAME_LEN);
    params_a.scale_len = 1;
    params_a.scale = (float *)malloc(params_a.scale_len * sizeof(float));
    params_a.scale[0] = 0.2;
    params_a.zp_len = 1;
    params_a.zp = (int32_t *)malloc(params_a.zp_len * sizeof(int32_t));
    params_a.zp[0] = 0;
    rknn_matmul_set_quant_params(ctx, &params_a);
    free(params_a.scale);
    free(params_a.zp);

    if (info.B_quant_type == 1)
    {
      rknn_quant_params params_b;
      memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
      params_b.scale_len = maxN;
      params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
      for (int i = 0; i < params_b.scale_len; i++)
        params_b.scale[i] = 0.1;
      params_b.zp_len = maxN;
      params_b.zp = (int32_t *)malloc(params_b.zp_len * sizeof(int32_t));
      memset(params_b.zp, 0, sizeof(int32_t) * params_b.zp_len);
      rknn_matmul_set_quant_params(ctx, &params_b);
      free(params_b.scale);
      free(params_b.zp);
    }
    else
    {
      rknn_quant_params params_b;
      memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
      params_b.scale_len = 1;
      params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
      params_b.scale[0] = 0.1;
      params_b.zp_len = 1;
      params_b.zp = (int32_t *)malloc(params_b.zp_len * sizeof(int32_t));
      params_b.zp[0] = 0;
      rknn_matmul_set_quant_params(ctx, &params_b);
      free(params_b.scale);
      free(params_b.zp);
    }
  }

  if (matmul_type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32 || matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 ||
      matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 || matmul_type == RKNN_INT8_MM_INT8_TO_FLOAT32)
  {
    if (info.B_quant_type == RKNN_QUANT_TYPE_PER_CHANNEL_SYM)
    {
      rknn_quant_params params_b;
      memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
      params_b.scale_len = maxN;
      params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
      for (int i = 0; i < params_b.scale_len; i++)
        params_b.scale[i] = 0.1;
      params_b.zp_len = maxN;
      params_b.zp = (int32_t *)malloc(params_b.zp_len * sizeof(int32_t));
      memset(params_b.zp, 0, sizeof(int32_t) * params_b.zp_len);
      rknn_matmul_set_quant_params(ctx, &params_b);
      free(params_b.scale);
      free(params_b.zp);
    }
    else if (info.B_quant_type == RKNN_QUANT_TYPE_PER_GROUP_SYM)
    {
      rknn_quant_params params_b;
      memset(&params_b, 0, sizeof(rknn_quant_params));
      memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
      params_b.scale_len = maxN * maxK / info.group_size;
      params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
      for (int i = 0; i < maxK / info.group_size; i++)
        for (int j = 0; j < maxN; j++)
          params_b.scale[i * maxN + j] = 0.1;
      rknn_matmul_set_quant_params(ctx, &params_b);
      free(params_b.scale);
    }
    else
    {
      rknn_quant_params params_b;
      memcpy(params_b.name, io_attr[0].B.name, RKNN_MAX_NAME_LEN);
      params_b.scale_len = 1;
      params_b.scale = (float *)malloc(params_b.scale_len * sizeof(float));
      params_b.scale[0] = 0.1;
      params_b.zp_len = 1;
      params_b.zp = (int32_t *)malloc(params_b.zp_len * sizeof(int32_t));
      params_b.zp[0] = 0;
      rknn_matmul_set_quant_params(ctx, &params_b);
      free(params_b.scale);
      free(params_b.zp);
    }
  }

  // malloc A/B/C buffer
  void *A_Matrix = nullptr;
  void *B_Matrix = nullptr;
  void *B_Matrix_ = nullptr;
  void *C_Matrix = nullptr;
  int A_type_bytes = 1;
  int B_type_bytes = 1;
  if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
  {
    A_type_bytes = 1;
    B_type_bytes = 1;
    A_Matrix = malloc(maxM * maxK);
    B_Matrix = malloc(maxK * maxN);
    B_Matrix_ = malloc(maxK * maxN);
    C_Matrix = malloc(maxM * maxN * sizeof(int16_t));

    // generate int4 A buffer
    int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(A_int8_Matrix, maxM * maxK, {-8, 7});
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else if (info.type == RKNN_INT8_MM_INT8_TO_INT32 || info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
  {
    A_type_bytes = 1;
    B_type_bytes = 1;
    A_Matrix = malloc(maxM * maxK);
    B_Matrix = malloc(maxK * maxN);
    C_Matrix = malloc(maxM * maxN * sizeof(int32_t));

    // generate int8 A buffer
    int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(A_int8_Matrix, maxM * maxK, {-128, 127});
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-128, 127});
  }
  else if (info.type == RKNN_INT8_MM_INT8_TO_INT8)
  {
    A_type_bytes = 1;
    B_type_bytes = 1;
    A_Matrix = malloc(maxM * maxK);
    B_Matrix = malloc(maxK * maxN);
    C_Matrix = malloc(maxM * maxN * sizeof(int8_t));

    // generate int8 A buffer
    int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(A_int8_Matrix, maxM * maxK, {5, 6});
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {10, 11});
  }
  else if (info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16 || info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32)
  {
    A_type_bytes = 2;
    B_type_bytes = 2;
    A_Matrix = malloc(maxM * maxK * A_type_bytes);
    B_Matrix = malloc(maxK * maxN * B_type_bytes);
    C_Matrix = malloc(maxM * maxN * sizeof(float));

    // generate int16 A buffer
    float16 *A_float16_Matrix = (float16 *)A_Matrix;
    float16 *B_float16_Matrix = (float16 *)B_Matrix;
    generate_random_buffer(A_float16_Matrix, maxM * maxK, {-1.f, 1.f});
    generate_random_buffer(B_float16_Matrix, maxK * maxN, {-1.f, 1.f});
  }
  else if (info.type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 ||
           info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32)
  {
    A_type_bytes = 2;
    B_type_bytes = 1;
    A_Matrix = malloc(maxM * maxK * A_type_bytes);
    B_Matrix = malloc(maxK * maxN);
    B_Matrix_ = malloc(maxK * maxN);
    C_Matrix = malloc(maxM * maxN * sizeof(float));

    float16 *A_float16_Matrix = (float16 *)A_Matrix;
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(A_float16_Matrix, maxM * maxK, {-1.f, 1.f});
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else if (info.type == RKNN_INT8_MM_INT4_TO_INT32)
  {
    A_type_bytes = 1;
    B_type_bytes = 1;
    A_Matrix = malloc(maxM * maxK * A_type_bytes);
    B_Matrix = malloc(maxK * maxN);
    B_Matrix_ = malloc(maxK * maxN);
    C_Matrix = malloc(maxM * maxN * sizeof(int32_t));

    int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(A_int8_Matrix, maxM * maxK, {-128, 127});
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else
  {
    fprintf(stderr, "unsupported data type: %d\n", info.type);
    return -1;
  }

  // Create A
  int max_a_size = io_attr[0].A.size;
  for (int i = 1; i < shapes.size(); i++)
  {
    if (io_attr[i].A.size > max_a_size)
    {
      max_a_size = io_attr[i].A.size;
    }
  }
  rknn_tensor_mem *A = rknn_create_mem(ctx, max_a_size);
  if (A == NULL)
  {
    fprintf(stderr, "rknn_create_mem fail!\n");
    return -1;
  }

  // Create B
  int max_b_size = io_attr[0].B.size;
  for (int i = 1; i < shapes.size(); i++)
  {
    if (io_attr[i].B.size > max_b_size)
    {
      max_b_size = io_attr[i].B.size;
    }
  }
  rknn_tensor_mem *B = rknn_create_mem(ctx, max_b_size);
  if (B == NULL)
  {
    fprintf(stderr, "rknn_create_mem fail!\n");
    return -1;
  }

  // Create C
  int max_c_size = io_attr[0].C.size;
  for (int i = 1; i < shapes.size(); i++)
  {
    if (io_attr[i].C.size > max_c_size)
    {
      max_c_size = io_attr[i].C.size;
    }
  }
  rknn_tensor_mem *C = rknn_create_mem(ctx, max_c_size);
  if (C == NULL)
  {
    fprintf(stderr, "rknn_create_mem fail!\n");
    return -1;
  }

  // random B data
  if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
  {
    // generate int4 A buffer
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else if (info.type == RKNN_INT8_MM_INT8_TO_INT32 || info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
  {
    // generate int8 A buffer
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-128, 127});
  }
  else if (info.type == RKNN_INT8_MM_INT8_TO_INT8)
  {
    // generate int8 A buffer
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {10, 11});
  }
  else if (info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32 || info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16)
  {
    // generate int16 A buffer
    float16 *B_float16_Matrix = (float16 *)B_Matrix;
    generate_random_buffer(B_float16_Matrix, maxK * maxN, {-1.f, 1.f});
  }
  else if (info.type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 ||
           info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32)
  {
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else if (info.type == RKNN_INT8_MM_INT4_TO_INT32)
  {
    int8_t *B_int8_Matrix = (int8_t *)B_Matrix;
    generate_random_buffer(B_int8_Matrix, maxK * maxN, {-8, 7});
  }
  else
  {
    fprintf(stderr, "unsupported data type: %d\n", info.type);
    return -1;
  }

  // B matrix
  // normal layout
  uint32_t bii = 0;
  if (info.B_layout == 0)
  {
    if (info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 ||
        info.type == RKNN_INT8_MM_INT4_TO_INT32 || info.type == RKNN_INT4_MM_INT4_TO_INT16)
    {
      int size = io_attr[0].B.dims[1] * io_attr[0].B.dims[0];
      set_mem_from_int8_to_int4((int8_t *)B->virt_addr, (int8_t *)B_Matrix, size);
    }
    else
    {
      memcpy(B->virt_addr, B_Matrix, maxK * maxN * B_type_bytes);
    }
  }
  else if (info.B_layout == 1)
  {
    // native layout: [N1, K1, subN, subK]
    int32_t subN = io_attr[0].B.dims[2];
    int32_t subK = io_attr[0].B.dims[3];
    std::vector<int> input_shape = {(int)(maxK / subK), subK, int(maxN / subN), subN};
    std::vector<int> output_shape = {int(maxN / subN), (int)(maxK / subK), subN, subK};
    if (info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 ||
        info.type == RKNN_INT8_MM_INT4_TO_INT32 || info.type == RKNN_INT4_MM_INT4_TO_INT16)
    {
      int size = io_attr[0].B.dims[1] * io_attr[0].B.dims[0];
      memcpy(B_Matrix_, B_Matrix, maxK * maxN * B_type_bytes);
      set_mem_from_int8_to_int4((int8_t *)B_Matrix_, (int8_t *)B_Matrix, maxK * maxN);
      rknn_B_normal_layout_to_native_layout(B_Matrix_, B->virt_addr, maxK, maxN, &info);
    }
    else
    {
      rknn_B_normal_layout_to_native_layout(B_Matrix, B->virt_addr, maxK, maxN, &info);
    }
  }
  // rknn_matmul_set_io_mem must call after rknn_matmul_set_dynamic_shape
  ret = rknn_matmul_set_dynamic_shape(ctx, &shapes.at(0));
  if (ret != 0)
  {
    fprintf(stderr, "rknn_matmul_set_dynamic_shapes fail!\n");
    return -1;
  }

  // B can not be dynamic, only call set_io_mem once.
  ret = rknn_matmul_set_io_mem(ctx, B, &io_attr[0].B);
  if (ret < 0)
  {
    fprintf(stderr, "rknn_matmul_set_io_mem fail! ret=%d\n", ret);
    return -1;
  }

  for (int s = 0; s < shapes.size(); s++)
  {
    // set A shape
    int M = shapes[s].M;
    int K = shapes[s].K;
    int N = shapes[s].N;

    ret = rknn_matmul_set_dynamic_shape(ctx, &shapes.at(s));
    if (ret != 0)
    {
      fprintf(stderr, "rknn_matmul_set_dynamic_shapes fail!\n");
      return -1;
    }

    printf("input/output matmul current tensor attribute:\n");
    dump_matmul_tensor_attr(&io_attr[s].A);
    dump_matmul_tensor_attr(&io_attr[s].B);
    dump_matmul_tensor_attr(&io_attr[s].C);

    // random A data
    if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
    {
      // generate int4 A buffer
      int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
      generate_random_buffer(A_int8_Matrix, M * K, {-8, 7});
    }
    else if (info.type == RKNN_INT8_MM_INT8_TO_INT32 || info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
    {
      // generate int8 A buffer
      int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
      generate_random_buffer(A_int8_Matrix, M * K, {-128, 127});
    }
    else if (info.type == RKNN_INT8_MM_INT8_TO_INT8)
    {
      // generate int8 A buffer
      int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
      generate_random_buffer(A_int8_Matrix, M * K, {5, 6});
    }
    else if (info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32 || info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16)
    {
      // generate int16 A buffer
      float16 *A_float16_Matrix = (float16 *)A_Matrix;
      generate_random_buffer(A_float16_Matrix, M * K, {-1.f, 1.f});
    }
    else if (info.type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 ||
             info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32)
    {
      float16 *A_float16_Matrix = (float16 *)A_Matrix;
      generate_random_buffer(A_float16_Matrix, M * K, {-1.f, 1.f});
    }
    else if (info.type == RKNN_INT8_MM_INT4_TO_INT32)
    {
      int8_t *A_int8_Matrix = (int8_t *)A_Matrix;
      generate_random_buffer(A_int8_Matrix, M * K, {-128, 127});
    }
    else
    {
      fprintf(stderr, "unsupported data type: %d\n", info.type);
      return -1;
    }

    // A matrix
    // normal layout
    uint32_t aii = 0;
    if (info.AC_layout == 0)
    {
      if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
      {
        int size = io_attr[s].A.dims[1] * io_attr[s].A.dims[0];
        set_mem_from_int8_to_int4((int8_t *)A->virt_addr, (int8_t *)A_Matrix, size);
      }
      else
      {
        memcpy(A->virt_addr, A_Matrix, M * K * A_type_bytes);
      }
    }
    else
    {
      //  perf  layout: [K1, M, subK]
      int32_t subK = io_attr[s].A.dims[2];
      if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
      {
        norm_layout_to_perf_layout<int8_t, int8_t>(static_cast<int8_t *>(A_Matrix), static_cast<int8_t *>(A->virt_addr),
                                                   M, K, subK, true);
      }
      else if (info.type == RKNN_INT8_MM_INT8_TO_INT32 || info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
      {
        norm_layout_to_perf_layout<int8_t, int8_t>(static_cast<int8_t *>(A_Matrix), static_cast<int8_t *>(A->virt_addr),
                                                   M, K, subK, false);
      }
      else
      {
        norm_layout_to_perf_layout<float16, float16>(static_cast<float16 *>(A_Matrix),
                                                     static_cast<float16 *>(A->virt_addr), M, K, subK, false);
      }
    }

    // Set A
    ret = rknn_matmul_set_io_mem(ctx, A, &io_attr[s].A);
    if (ret < 0)
    {
      fprintf(stderr, "rknn_matmul_set_io_mem fail! ret=%d\n", ret);
      return -1;
    }

    if (info.B_layout == RKNN_MM_LAYOUT_TP_NORM)
    {
      // B_Matrix is [K, maxN], which needs to be converted to [N, K]
      if (info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16 || info.type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32 ||
          info.type == RKNN_INT8_MM_INT4_TO_INT32 || info.type == RKNN_INT4_MM_INT4_TO_INT16)
      {
        // transpose int4 B
        transpose4bit((const int8_t *)B_Matrix, (int8_t *)B->virt_addr, K, N);
      }
      else if ((info.type == RKNN_INT8_MM_INT8_TO_INT32) || (info.type == RKNN_INT8_MM_INT8_TO_INT8) ||
               (info.type == RKNN_INT8_MM_INT4_TO_INT32) || info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
      {
        transposeB<int8_t>(static_cast<const int8_t *>(B_Matrix), static_cast<int8_t *>(B->virt_addr), K, N);
      }
      else
      {
        transposeB<float16>(static_cast<const float16 *>(B_Matrix), static_cast<float16 *>(B->virt_addr), K, N);
      }

      // Set B
      ret = rknn_matmul_set_io_mem(ctx, B, &io_attr[s].B);
      if (ret < 0)
      {
        fprintf(stderr, "rknn_matmul_set_io_mem fail! ret=%d\n", ret);
        return -1;
      }
    }

    // Set C
    ret = rknn_matmul_set_io_mem(ctx, C, &io_attr[s].C);
    if (ret < 0)
    {
      fprintf(stderr, "rknn_matmul_set_io_mem fail! ret=%d\n", ret);
      return -1;
    }

    // Run
    printf("Begin perf ...\n");
    for (int i = 0; i < loop_count; ++i)
    {
      int64_t start_us = getCurrentTimeUs();
      ret = rknn_matmul_run(ctx);
      int64_t elapse_us = getCurrentTimeUs() - start_us;
      if (ret < 0)
      {
        printf("rknn_matmul_run error %d\n", ret);
        return -1;
      }
      printf("%4d: Elapse Time = %.2fms, FPS = %.2f\n", i, elapse_us / 1000.f, 1000.f * 1000.f / elapse_us);
    }

    // Dump A/B/C tensors
    if (print_tensor != 0)
    {
      printf("matmul tensors:\n");
      dump_matmul_tensor(A, &io_attr[s].A);
      dump_matmul_tensor(B, &io_attr[s].B);
#if DUMP_REVERSE_WEIGHT
      // Dump the two axes of weight in reverse so that it is easy to multiply them by the input one-to-one
      dump_matmul_tensor_reverse(B, &io_attr.B);
#endif
      dump_matmul_tensor(C, &io_attr[s].C);
    }

    // compare NPU res vs CPU res
    if (info.type == RKNN_INT4_MM_INT4_TO_INT16)
    {
      int16_t *npu_res_ptr = (int16_t *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (int16_t *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (int16_t *)C_Matrix;
      }
      std::vector<int16_t> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      std::vector<int16_t> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiply<int8_t, int16_t>((const int8_t *)A_Matrix, (const int8_t *)B_Matrix, M, K, N);

      if (arraysEqual<int16_t>(cpu_res, npu_res))
      {
        printf("INT4_MM_INT4_TO_INT16 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf("INT4_MM_INT4_TO_INT16 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
               K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (info.type == RKNN_INT8_MM_INT8_TO_INT32)
    {
      int32_t *npu_res_ptr = (int32_t *)C->virt_addr;
      // rknn_mem_sync(0, C, RKNN_MEMORY_SYNC_FROM_DEVICE);
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (int32_t *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (int32_t *)C_Matrix;
      }
      std::vector<int32_t> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      std::vector<int32_t> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiply<int8_t, int32_t>((const int8_t *)A_Matrix, (const int8_t *)B_Matrix, M, K, N);

      if (arraysEqual<int32_t>(cpu_res, npu_res))
      {
        printf("INT8_MM_INT8_TO_INT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
        ret = 0;
      }
      else
      {
        printf("INT8_MM_INT8_TO_INT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
               K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (info.type == RKNN_INT8_MM_INT8_TO_FLOAT32)
    {
      float *npu_res_ptr = (float *)C->virt_addr;
      // rknn_mem_sync(0, C, RKNN_MEMORY_SYNC_FROM_DEVICE);
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 1.0;
      float weight_scale = 0.0001;
      float output_scale = 1.0;
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithHybridQuant<int8_t, int8_t, float>((const int8_t *)A_Matrix, (const int8_t *)B_Matrix,
                                                                     M, K, N, input_scale, weight_scale, output_scale);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("INT8_MM_INT8_TO_FLOAT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
        ret = 0;
      }
      else
      {
        printf("INT8_MM_INT8_TO_FLOAT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
               K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT32)
    {
      float *npu_res_ptr = (float *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      // calculate cpu res
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiply<float16, float>((const float16 *)A_Matrix, (const float16 *)B_Matrix, M, K, N);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("FLOAT16_MM_FLOAT16_TO_FLOAT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
        ret = 0;
      }
      else
      {
        printf("FLOAT16_MM_FLOAT16_TO_FLOAT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (info.type == RKNN_FLOAT16_MM_FLOAT16_TO_FLOAT16)
    {
      float16 *npu_res_ptr = (float16 *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float16 *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float16 *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      // calculate cpu res
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiply<float16, float>((const float16 *)A_Matrix, (const float16 *)B_Matrix, M, K, N);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("FLOAT16_MM_FLOAT16_TO_FLOAT16 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf("FLOAT16_MM_FLOAT16_TO_FLOAT16 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (info.type == RKNN_INT8_MM_INT8_TO_INT8)
    {
      int8_t *npu_res_ptr = (int8_t *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (int8_t *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (int8_t *)C_Matrix;
      }
      std::vector<int8_t> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 0.2;
      float weight_scale = 0.1;
      float output_scale = 0.8;
      std::vector<int8_t> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithQuant((const int8_t *)A_Matrix, (const int8_t *)B_Matrix, M, K, N, input_scale,
                                        weight_scale, output_scale);

      if (arraysEqual<int8_t>(cpu_res, npu_res))
      {
        printf("INT8_MM_INT8_TO_INT8 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf("INT8_MM_INT8_TO_INT8 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
               K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (matmul_type == RKNN_FLOAT16_MM_INT8_TO_FLOAT32)
    {
      float *npu_res_ptr = (float *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 1;
      float weight_scale = 0.1;
      float output_scale = 1;
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithHybridQuant<float16, int8_t, float>((const float16 *)A_Matrix, (const int8_t *)B_Matrix,
                                                                      M, K, N, input_scale, weight_scale, output_scale);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("FLOAT16_MM_INT8_TO_FLOAT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf(
            "FLOAT16_MM_INT8_TO_FLOAT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
            K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT16)
    {
      float16 *npu_res_ptr = (float16 *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float16 *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float16 *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 1;
      float weight_scale = 0.1;
      float output_scale = 1;
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithHybridQuant<float16, int8_t, float>((const float16 *)A_Matrix, (const int8_t *)B_Matrix,
                                                                      M, K, N, input_scale, weight_scale, output_scale);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("FLOAT16_MM_INT4_TO_FLOAT16 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf(
            "FLOAT16_MM_INT4_TO_FLOAT16 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
            K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (matmul_type == RKNN_FLOAT16_MM_INT4_TO_FLOAT32)
    {
      float *npu_res_ptr = (float *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (float *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (float *)C_Matrix;
      }
      std::vector<float> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 1;
      float weight_scale = 0.1;
      float output_scale = 1;
      std::vector<float> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithHybridQuant<float16, int8_t, float>((const float16 *)A_Matrix, (const int8_t *)B_Matrix,
                                                                      M, K, N, input_scale, weight_scale, output_scale);

      if (arraysCosineSimilarity<float>(cpu_res, npu_res))
      {
        printf("FLOAT16_MM_INT4_TO_FLOAT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout "
               "is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf(
            "FLOAT16_MM_INT4_TO_FLOAT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
            K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
    else if (matmul_type == RKNN_INT8_MM_INT4_TO_INT32)
    {
      int32_t *npu_res_ptr = (int32_t *)C->virt_addr;
      if (info.AC_layout == 1)
      {
        int32_t N_remain = io_attr[s].C.dims[0];
        int32_t subN = io_attr[s].C.dims[2];
        perf_layout_to_norm_layout(npu_res_ptr, (int32_t *)C_Matrix, M, N, N_remain, subN);
        npu_res_ptr = (int32_t *)C_Matrix;
      }
      std::vector<int32_t> npu_res(npu_res_ptr, npu_res_ptr + M * N);

      float input_scale = 0.2;
      float weight_scale = 0.1;
      float output_scale = 1;
      std::vector<int32_t> cpu_res;
      cpu_res.reserve(M * N);
      cpu_res = matrixMultiplyWithHybridQuant<int8_t, int8_t, int32_t>(
          (const int8_t *)A_Matrix, (const int8_t *)B_Matrix, M, K, N, input_scale, weight_scale, output_scale);

      if (arraysCosineSimilarity<int32_t>(cpu_res, npu_res))
      {
        printf("INT8_MM_INT4_TO_INT32 matmul result is correct M x K x N is %d %d %d AC_layout is %d B_layout is %d\n",
               M, K, N, AC_layout, B_layout);
        ret = 0;
      }
      else
      {
        printf("INT8_MM_INT4_TO_INT32 matmul result is wrong M x K x N is %d %d %d AC_layout is %d B_layout is %d\n", M,
               K, N, AC_layout, B_layout);
        ret = -1;
      }
    }
  }

  // destroy
  rknn_destroy_mem(ctx, A);
  rknn_destroy_mem(ctx, B);
  rknn_destroy_mem(ctx, C);

  rknn_matmul_destroy(ctx);

  // clean data
  if (A_Matrix)
  {
    free(A_Matrix);
  }
  if (B_Matrix)
  {
    free(B_Matrix);
  }
  if (C_Matrix)
  {
    free(C_Matrix);
  }

  return ret;
}