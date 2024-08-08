// Copyright (c) 2023 by Rockchip Electronics Co., Ltd. All Rights Reserved.
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

#include <string>
#include <vector>

#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "fp16/Float16.h"
#include "rknn_api.h"
#include "rknn_custom_op.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define LOAD_FROM_PATH 

extern "C" {
RKNN_CUSTOM_OP_EXPORT rknn_custom_op* get_rknn_custom_op();
}


/*-------------------------------------------
                  Functions
-------------------------------------------*/
static inline int64_t getCurrentTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}


static void compose_img(float *res_buf, unsigned char *img_buf, const int height, const int width)
{
  for (int h = 0; h < height; ++h)
  {
    for (int w = 0; w < width; ++w)
    {
      auto pixel = res_buf[h * width + w];
      if (pixel > 0.f) {
        img_buf[h * width * 3 + w * 3] = 0;
        img_buf[h * width * 3 + w * 3 + 1] = 0;
        img_buf[h * width * 3 + w * 3 + 2] = 0;
      }
    }
  }

  constexpr char out_path[] = "test_compose_img.png"; 
  // write out image using stb 
  printf("Writing out composed image with: %dx%d\n", width, height);
  stbi_write_png(out_path, width, height, 3, img_buf, 0);
}

static void dump_tensor_attr(rknn_tensor_attr* attr)
{
  std::string shape_str = attr->n_dims < 1 ? "" : std::to_string(attr->dims[0]);
  for (int i = 1; i < attr->n_dims; ++i) {
    shape_str += ", " + std::to_string(attr->dims[i]);
  }

  printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride = %d, size_with_stride=%d, fmt=%s, "
         "type=%s, qnt_type=%s, "
         "zp=%d, scale=%f\n",
         attr->index, attr->name, attr->n_dims, shape_str.c_str(), attr->n_elems, attr->size, attr->w_stride,
         attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
         get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}

static int get_type_bytes(rknn_tensor_type type)
{
  switch (type) {
  case RKNN_TENSOR_INT64:
    return 8;
  case RKNN_TENSOR_UINT32:;
  case RKNN_TENSOR_INT32:
  case RKNN_TENSOR_FLOAT32:
    return 4;
  case RKNN_TENSOR_INT16:
  case RKNN_TENSOR_UINT16:;
  case RKNN_TENSOR_FLOAT16:
    return 2;
  case RKNN_TENSOR_BOOL:
  case RKNN_TENSOR_INT8:
  case RKNN_TENSOR_UINT8:
    return 1;
  default:
    return 0;
  }
}

static unsigned char* load_image(const char* image_path, rknn_tensor_attr* input_attr)
{
  int req_height  = 0;
  int req_width   = 0;
  int req_channel = 0;

  switch (input_attr->fmt) {
  case RKNN_TENSOR_NHWC:
    req_height  = input_attr->dims[1];
    req_width   = input_attr->dims[2];
    req_channel = input_attr->dims[3];
    break;
  case RKNN_TENSOR_NCHW:
    req_height  = input_attr->dims[2];
    req_width   = input_attr->dims[3];
    req_channel = input_attr->dims[1];
    break;
  default:
    printf("meet unsupported layout\n");
    return NULL;
  }

  int height  = 0;
  int width   = 0;
  int channel = 0;

  unsigned char* image_data = stbi_load(image_path, &width, &height, &channel, req_channel);
  if (image_data == NULL) {
    printf("load image failed!\n");
    return NULL;
  }

  if (width != req_width || height != req_height) {
    unsigned char* image_resized = (unsigned char*)STBI_MALLOC(req_width * req_height * req_channel);
    if (!image_resized) {
      printf("malloc image failed!\n");
      STBI_FREE(image_data);
      return NULL;
    }
    if (stbir_resize_uint8(image_data, width, height, 0, image_resized, req_width, req_height, 0, channel) != 1) {
      printf("resize image failed!\n");
      STBI_FREE(image_data);
      return NULL;
    }
    STBI_FREE(image_data);
    image_data = image_resized;
  }

  return image_data;
}

static std::vector<std::string> split(const std::string& str, const std::string& pattern)
{
  std::vector<std::string> res;
  if (str == "")
    return res;
  std::string strs = str + pattern;
  size_t      pos  = strs.find(pattern);
  while (pos != strs.npos) {
    std::string temp = strs.substr(0, pos);
    res.push_back(temp);
    strs = strs.substr(pos + 1, strs.size());
    pos  = strs.find(pattern);
  }
  return res;
}

static int write_data_to_file(char* path, char* data, unsigned int size)
{
  FILE* fp;

  fp = fopen(path, "w");
  if (fp == NULL) {
    printf("open error: %s", path);
    return -1;
  }

  fwrite(data, 1, size, fp);
  fflush(fp);

  fclose(fp);
  return 0;
}

static void* load_file(const char* file_path, size_t* file_size)
{
  FILE* fp = fopen(file_path, "r");
  if (fp == NULL) {
    printf("failed to open file: %s\n", file_path);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  size_t size = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);

  void* file_data = malloc(size);
  if (file_data == NULL) {
    fclose(fp);
    printf("failed allocate file size: %zu\n", size);
    return NULL;
  }

  if (fread(file_data, 1, size, fp) != size) {
    fclose(fp);
    free(file_data);
    printf("failed to read file data!\n");
    return NULL;
  }

  fclose(fp);

  *file_size = size;

  return file_data;
}


/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
int main(int argc, char* argv[])
{
  if (argc < 2) {
    printf("Usage:%s model_path [input_path] [loop_count] \n", argv[0]);
    return -1;
  }

  char* model_path = argv[1];

  std::vector<std::string> input_paths_split;
  if (argc > 2) {
    char* input_paths = argv[2];
    input_paths_split = split(input_paths, "#");
  }

  int loop_count = 1;
  if (argc > 3) {
    loop_count = atoi(argv[3]);
  }

  char* output_dir = NULL;
  if (argc > 6) {
    output_dir = argv[6];
  }

  rknn_context ctx = 0;

  // Load RKNN Model
  uint32_t flag = 0;
  flag          = flag | RKNN_FLAG_EXECUTE_FALLBACK_PRIOR_DEVICE_GPU;
#ifdef LOAD_FROM_PATH
  // Init rknn from model path
  int ret = rknn_init(&ctx, model_path, 0, flag, NULL);
#else
  // Init rknn from model data
  size_t model_size;
  printf("load model from buffer.\n");
  void* model_data = load_file(model_path, &model_size);
  if (model_data == NULL) {
    return -1;
  }
  int ret = rknn_init(&ctx, model_data, model_size, 0, NULL);
  free(model_data);
#endif
  if (ret < 0) {
    printf("rknn_init fail! ret=%d\n", ret);
    return -1;
  }

  //user op already delcared on other files.
  auto gpu_user_op = get_rknn_custom_op();
  ret = rknn_register_custom_ops(ctx, gpu_user_op, 1);
  if (ret < 0) {
    printf("rknn_register_custom_ops fail! ret = %d\n", ret);
    return -1;
  }

  // Get sdk and driver version
  rknn_sdk_version sdk_ver;
  ret = rknn_query(ctx, RKNN_QUERY_SDK_VERSION, &sdk_ver, sizeof(sdk_ver));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("rknn_api/rknnrt version: %s, driver version: %s\n", sdk_ver.api_version, sdk_ver.drv_version);
  rknn_mem_size mem_size;
  ret = rknn_query(ctx, RKNN_QUERY_MEM_SIZE, &mem_size, sizeof(mem_size));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("total weight size: %u, total internal size: %u\n", mem_size.total_weight_size, mem_size.total_internal_size);
  printf("total dma used size: %zu\n", (size_t)mem_size.total_dma_allocated_size);

  // Get Model Input Output Info
  rknn_input_output_num io_num;
  ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output);

  printf("input tensors:\n");
  rknn_tensor_attr input_attrs[io_num.n_input];
  memset(input_attrs, 0, io_num.n_input * sizeof(rknn_tensor_attr));
  for (uint32_t i = 0; i < io_num.n_input; i++) {
    input_attrs[i].index = i;
    // query info
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
    if (ret < 0) {
      printf("rknn_query error! ret=%d\n", ret);
      return -1;
    }
    dump_tensor_attr(&input_attrs[i]);
  }

  printf("output tensors:\n");
  rknn_tensor_attr output_attrs[io_num.n_output];
  memset(output_attrs, 0, io_num.n_output * sizeof(rknn_tensor_attr));
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    output_attrs[i].index = i;
    // query info
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC) {
      printf("rknn_query fail! ret=%d\n", ret);
      return -1;
    }
    dump_tensor_attr(&output_attrs[i]);
  }

  // Get custom string
  rknn_custom_string custom_string;
  ret = rknn_query(ctx, RKNN_QUERY_CUSTOM_STRING, &custom_string, sizeof(custom_string));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("custom string: %s\n", custom_string.string);

  unsigned char* input_data[io_num.n_input];
  int            input_type[io_num.n_input];
  int            input_layout[io_num.n_input];
  int            input_size[io_num.n_input];
  for (int i = 0; i < io_num.n_input; i++) {
    input_data[i]   = NULL;
    input_type[i]   = RKNN_TENSOR_UINT8;
    input_layout[i] = input_attrs[i].fmt;
    input_size[i]   = input_attrs[i].n_elems * sizeof(uint8_t);
  }

  if (input_paths_split.size() > 0) {
    // Load input
    if (io_num.n_input != input_paths_split.size()) {
      printf("input missing!, need input number: %d\n", io_num.n_input);
      return -1;
    }
    for (int i = 0; i < io_num.n_input; i++) {
      // Load image
      input_data[i] = load_image(input_paths_split[i].c_str(), &input_attrs[i]);

      if (!input_data[i]) {
        return -1;
      }
    }
  } else {
    for (int i = 0; i < io_num.n_input; i++) {
      input_data[i] = (unsigned char*)malloc(input_size[i]);
    }
  }

  rknn_input inputs[io_num.n_input];
  memset(inputs, 0, io_num.n_input * sizeof(rknn_input));
  for (int i = 0; i < io_num.n_input; i++) {
    inputs[i].index        = i;
    inputs[i].pass_through = 0;
    inputs[i].type         = (rknn_tensor_type)input_type[i];
    inputs[i].fmt          = (rknn_tensor_format)input_layout[i];
    inputs[i].buf          = input_data[i];
    inputs[i].size         = input_size[i];
  }

  // Set input
  ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
  if (ret < 0) {
    printf("rknn_input_set fail! ret=%d\n", ret);
    return -1;
  }

  auto core_mask = RKNN_NPU_CORE_ALL;
  rknn_set_core_mask(ctx, (rknn_core_mask)core_mask);

  // Run
  printf("Begin perf ...\n");
  double total_time = 0;
  for (int i = 0; i < loop_count; ++i) {
    int64_t start_us  = getCurrentTimeUs();
    ret               = rknn_run(ctx, NULL);
    int64_t elapse_us = getCurrentTimeUs() - start_us; if (ret < 0) {
      printf("rknn run error %d\n", ret);
      return -1;
    }
    total_time += elapse_us / 1000.f;
    printf("%4d: Elapse Time = %.2fms, FPS = %.2f\n", i, elapse_us / 1000.f, 1000.f * 1000.f / elapse_us);
  }
  printf("Avg elapse Time = %.3fms\n", total_time / loop_count);
  printf("Avg FPS = %.3f\n", loop_count * 1000.f / total_time);

  // Get perf detail
  rknn_perf_detail perf_detail;
  ret = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail, sizeof(perf_detail));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("rknn run perf detail is:\n%s", perf_detail.perf_data);

  // Get run duration time
  rknn_perf_run perf_run;
  ret = rknn_query(ctx, RKNN_QUERY_PERF_RUN, &perf_run, sizeof(perf_run));
  if (ret != RKNN_SUCC) {
    printf("rknn_query fail! ret=%d\n", ret);
    return -1;
  }
  printf("rknn run perf time is %ldus\n", perf_run.run_duration);

  // Get output
  rknn_output outputs[io_num.n_output];
  memset(outputs, 0, io_num.n_output * sizeof(rknn_output));
  for (uint32_t i = 0; i < io_num.n_output; ++i) {
    outputs[i].want_float  = 1;
    outputs[i].index       = i;
    outputs[i].is_prealloc = 0;
  }

  ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
  if (ret < 0) {
    printf("rknn_outputs_get fail! ret=%d\n", ret);
    return ret;
  }

  auto out_width = output_attrs[0].dims[2];
  auto out_height = output_attrs[0].dims[1];
  compose_img((float* )outputs[0].buf, input_data[0], out_height, out_width);

  // release outputs
  ret = rknn_outputs_release(ctx, io_num.n_output, outputs);

  // destroy
  rknn_destroy(ctx);

  for (int i = 0; i < io_num.n_input; i++) {
    free(input_data[i]);
  }

  return 0;
}
