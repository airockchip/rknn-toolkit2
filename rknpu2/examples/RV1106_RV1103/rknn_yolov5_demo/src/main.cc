// Copyright (c) 2021 by Rockchip Electronics Co., Ltd. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "rknn_api.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>

#include "postprocess.h"

#define PERF_WITH_POST 1

/*-------------------------------------------
                  Functions
-------------------------------------------*/
static inline int64_t getCurrentTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}

static void dump_tensor_attr(rknn_tensor_attr *attr)
{
  char dims[128] = {0};
  for (int i = 0; i < attr->n_dims; ++i)
  {
    int idx = strlen(dims);
    sprintf(&dims[idx], "%d%s", attr->dims[i], (i == attr->n_dims - 1) ? "" : ", ");
  }
  printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, fmt=%s, type=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, dims, attr->n_elems, attr->size, get_format_string(attr->fmt),
         get_type_string(attr->type), get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

static void *load_file(const char *file_path, size_t *file_size)
{
  FILE *fp = fopen(file_path, "r");
  if (fp == NULL)
  {
    printf("failed to open file: %s\n", file_path);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  size_t size = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void *file_data = malloc(size);
  if (file_data == NULL)
  {
    fclose(fp);
    printf("failed allocate file size: %zu\n", size);
    return NULL;
  }

  if (fread(file_data, 1, size, fp) != size)
  {
    fclose(fp);
    free(file_data);
    printf("failed to read file data!\n");
    return NULL;
  }

  fclose(fp);

  *file_size = size;

  return file_data;
}

static unsigned char *load_image(const char *image_path, rknn_tensor_attr *input_attr, int *img_height, int *img_width)
{
  int req_height = 0;
  int req_width = 0;
  int req_channel = 0;

  switch (input_attr->fmt)
  {
  case RKNN_TENSOR_NHWC:
    req_height = input_attr->dims[1];
    req_width = input_attr->dims[2];
    req_channel = input_attr->dims[3];
    break;
  case RKNN_TENSOR_NCHW:
    req_height = input_attr->dims[2];
    req_width = input_attr->dims[3];
    req_channel = input_attr->dims[1];
    break;
  default:
    printf("meet unsupported layout\n");
    return NULL;
  }

  int channel = 0;

  unsigned char *image_data = stbi_load(image_path, img_width, img_height, &channel, req_channel);
  if (image_data == NULL)
  {
    printf("load image failed!\n");
    return NULL;
  }

  if (*img_width != req_width || *img_height != req_height)
  {
    unsigned char *image_resized = (unsigned char *)STBI_MALLOC(req_width * req_height * req_channel);
    if (!image_resized)
    {
      printf("malloc image failed!\n");
      STBI_FREE(image_data);
      return NULL;
    }
    if (stbir_resize_uint8(image_data, *img_width, *img_height, 0, image_resized, req_width, req_height, 0, channel) != 1)
    {
      printf("resize image failed!\n");
      STBI_FREE(image_data);
      return NULL;
    }
    STBI_FREE(image_data);
    image_data = image_resized;
  }

  return image_data;
}

int NC1HWC2_i8_to_NHWC_i8(const int8_t* src, int8_t* dst, int* dims, int channel, int h, int w)
{
  int batch  = dims[0];
  int C1     = dims[1];
  int C2     = dims[4];
  int hw_src = dims[2] * dims[3];
  int hw_dst = h * w;
  for (int i = 0; i < batch; i++) {
    const int8_t* src_b = src + i * C1 * hw_src * C2;
    int8_t*       dst_b = dst + i * channel * hw_dst;
    for (int cur_h = 0; cur_h < h; ++cur_h)
      for (int cur_w = 0; cur_w < w; ++cur_w) {
        for (int c = 0; c < channel; ++c) {
          int           plane  = c / C2;
          const int8_t* src_bc = plane * hw_src * C2 + src_b;
          int           offset = c % C2;
          int cur_hw                 = cur_h * w + cur_w;
          dst_b[cur_h * w * channel + cur_w* channel + c] = src_bc[C2 * cur_hw + offset] ;

        }
    }
  }
  return 0;
}


/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf("Usage:%s model_path input_path [loop_count]\n", argv[0]);
    return -1;
  }

  char *model_path = argv[1];
  char *input_path = argv[2];

  int loop_count = 1;
  if (argc > 3)
  {
    loop_count = atoi(argv[3]);
  }

  int64_t start_us, elapse_us;

  const float nms_threshold = NMS_THRESH;
  const float box_conf_threshold = BOX_THRESH;

  int img_width = 0;
  int img_height = 0;

  rknn_context ctx = 0;

  int model_width = 0;
  int model_height = 0;
  float scale_w,scale_h;
  detect_result_group_t detect_result_group;
  std::vector<float> out_scales;
  std::vector<int32_t> out_zps;

  // Load RKNN Model
#if 1
  // Init rknn from model path
  int ret = rknn_init(&ctx, model_path, 0, 0, NULL);
#else
  // Init rknn from model data
  size_t model_size;
  void *model_data = load_file(model_path, &model_size);
  if (model_data == NULL)
  {
    return -1;
  }
  int ret = rknn_init(&ctx, model_data, model_size, 0, NULL);
  free(model_data);
#endif
  if (ret < 0)
  {
    printf("rknn_init fail! ret=%d\n", ret);
    return -1;
  }

  // Get sdk and driver version
  rknn_sdk_version sdk_ver;
  ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &sdk_ver, sizeof(sdk_ver));
  if (ret != RKNN_SUCC)
  {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("rknn_api/rknnrt version: %s, driver version: %s\n", sdk_ver.api_version, sdk_ver.drv_version);

  // Get Model Input Output Info
  rknn_input_output_num io_num;
  ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (ret != RKNN_SUCC)
  {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

  printf("input tensors:\n");
  rknn_tensor_attr input_attrs[io_num.n_input];
  memset(input_attrs, 0, io_num.n_input * sizeof(rknn_tensor_attr));
  for (uint32_t i = 0; i < io_num.n_input; i++)
  {
    input_attrs[i].index = i;
    // query info
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
    if (ret < 0)
    {
      printf("rknn_init error! ret=%d\n", ret);
      return -1;
    }
    dump_tensor_attr(&input_attrs[i]);
  }

  printf("output tensors:\n");
  rknn_tensor_attr output_attrs[io_num.n_output];
  memset(output_attrs, 0, io_num.n_output * sizeof(rknn_tensor_attr));
  for (uint32_t i = 0; i < io_num.n_output; i++)
  {
    output_attrs[i].index = i;
    // query info
#if defined(RV1106_RV1103)
    ret = rknn_query(ctx, RKNN_QUERY_NATIVE_NHWC_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
#else // RV1106B_RV1103B
    ret = rknn_query(ctx, RKNN_QUERY_NATIVE_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
#endif
    if (ret != RKNN_SUCC)
    {
      printf("rknn_query fail! ret=%d\n", ret);
      return -1;
    }
    dump_tensor_attr(&output_attrs[i]);
  }

  // Get custom string
  rknn_custom_string custom_string;
  ret = rknn_query(ctx, RKNN_QUERY_CUSTOM_STRING, &custom_string, sizeof(custom_string));
  if (ret != RKNN_SUCC)
  {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("custom string: %s\n", custom_string.string);

  unsigned char *input_data = NULL;
  rknn_tensor_type input_type = RKNN_TENSOR_UINT8;
  rknn_tensor_format input_layout = RKNN_TENSOR_NHWC;
#if !defined(RV1106_RV1103)
  rknn_tensor_attr orig_output_attrs[io_num.n_output];
  int8_t* output_mems_nhwc[io_num.n_output];
#endif

  // Load image
  input_data = load_image(input_path, &input_attrs[0], &img_height, &img_width);

  if (!input_data)
  {
    return -1;
  }

  // Create input tensor memory
  rknn_tensor_mem *input_mems[1];
  // default input type is int8 (normalize and quantize need compute in outside)
  // if set uint8, will fuse normalize and quantize to npu
  input_attrs[0].type = input_type;
  // default fmt is NHWC, npu only support NHWC in zero copy mode
  input_attrs[0].fmt = input_layout;

  input_mems[0] = rknn_create_mem(ctx, input_attrs[0].size_with_stride);

  // Copy input data to input tensor memory
  int width = input_attrs[0].dims[2];
  int stride = input_attrs[0].w_stride;

  if (width == stride)
  {
    memcpy(input_mems[0]->virt_addr, input_data, width * input_attrs[0].dims[1] * input_attrs[0].dims[3]);
  }
  else
  {
    int height = input_attrs[0].dims[1];
    int channel = input_attrs[0].dims[3];
    // copy from src to dst with stride
    uint8_t *src_ptr = input_data;
    uint8_t *dst_ptr = (uint8_t *)input_mems[0]->virt_addr;
    // width-channel elements
    int src_wc_elems = width * channel;
    int dst_wc_elems = stride * channel;
    for (int h = 0; h < height; ++h)
    {
      memcpy(dst_ptr, src_ptr, src_wc_elems);
      src_ptr += src_wc_elems;
      dst_ptr += dst_wc_elems;
    }
  }

  // Create output tensor memory
  rknn_tensor_mem *output_mems[io_num.n_output];
  for (uint32_t i = 0; i < io_num.n_output; ++i)
  {
    output_mems[i] = rknn_create_mem(ctx, output_attrs[i].size_with_stride);
  }

  // Set input tensor memory
  ret = rknn_set_io_mem(ctx, input_mems[0], &input_attrs[0]);
  if (ret < 0)
  {
    printf("rknn_set_io_mem fail! ret=%d\n", ret);
    goto out;
  }

  // Set output tensor memory
  for (uint32_t i = 0; i < io_num.n_output; ++i)
  {
    // set output memory and attribute
    ret = rknn_set_io_mem(ctx, output_mems[i], &output_attrs[i]);
    if (ret < 0)
    {
      printf("rknn_set_io_mem fail! ret=%d\n", ret);
      goto out;
    }
  }

  // Run
  printf("Begin perf ...\n");
  for (int i = 0; i < loop_count; ++i)
  {
    start_us = getCurrentTimeUs();
    ret = rknn_run(ctx, NULL);
    elapse_us = getCurrentTimeUs() - start_us;
    if (ret < 0)
    {
      printf("rknn run error %d\n", ret);
      goto out;
    }
    printf("%4d: Elapse Time = %.2fms, FPS = %.2f\n", i, elapse_us / 1000.f, 1000.f * 1000.f / elapse_us);
  }

  if (input_attrs[0].fmt == RKNN_TENSOR_NCHW)
  {
    printf("model is NCHW input fmt\n");
    model_width = input_attrs[0].dims[2];
    model_height = input_attrs[0].dims[3];
  }
  else
  {
    printf("model is NHWC input fmt\n");
    model_width = input_attrs[0].dims[1];
    model_height = input_attrs[0].dims[2];
  }
  // post process
  scale_w = (float)model_width / img_width;
  scale_h = (float)model_height / img_height;

  for (int i = 0; i < io_num.n_output; ++i)
  {
    out_scales.push_back(output_attrs[i].scale);
    out_zps.push_back(output_attrs[i].zp);
  }

  start_us = getCurrentTimeUs();

#if defined(RV1106_RV1103)
    post_process((int8_t *)output_mems[0]->virt_addr, (int8_t *)output_mems[1]->virt_addr, (int8_t *)output_mems[2]->virt_addr, 
      model_height, model_width, box_conf_threshold, nms_threshold, scale_w, scale_h, out_zps, out_scales, &detect_result_group, NULL);

    
#else // RV1106B_RV1103B

    int dim[5*3];
    for (uint32_t i = 0; i < io_num.n_output; i++) {
      dim[5*i+0] =  (int)output_attrs[i].dims[0];
      dim[5*i+1] =  (int)output_attrs[i].dims[1];
      dim[5*i+2] =  (int)output_attrs[i].dims[2];
      dim[5*i+3] =  (int)output_attrs[i].dims[3];
      dim[5*i+4] =  (int)output_attrs[i].dims[4];
    }
    post_process((int8_t *)output_mems[0]->virt_addr, (int8_t *)output_mems[1]->virt_addr, (int8_t *)output_mems[2]->virt_addr, 
      model_height, model_width, box_conf_threshold, nms_threshold, scale_w, scale_h, out_zps, out_scales, &detect_result_group, dim);

#endif
   elapse_us = getCurrentTimeUs() - start_us;
   printf(" post_process Time = %.2fms, FPS = %.2f\n",  elapse_us / 1000.f, 1000.f * 1000.f / elapse_us);


  char text[256];
  for (int i = 0; i < detect_result_group.count; i++)
  {
    detect_result_t *det_result = &(detect_result_group.results[i]);
    sprintf(text, "%s %.1f%%", det_result->name, det_result->prop * 100);
    printf("%s @ (%d %d %d %d) %f\n",
           det_result->name,
           det_result->box.left, det_result->box.top, det_result->box.right, det_result->box.bottom,
           det_result->prop);
  }

out:
  // Destroy rknn memory
  rknn_destroy_mem(ctx, input_mems[0]);
  for (uint32_t i = 0; i < io_num.n_output; ++i)
  {
    rknn_destroy_mem(ctx, output_mems[i]);
  }

  // destroy
  rknn_destroy(ctx);

  free(input_data);

  return 0;
}
