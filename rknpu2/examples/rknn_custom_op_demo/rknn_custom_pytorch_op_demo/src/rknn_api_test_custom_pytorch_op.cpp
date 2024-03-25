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

#include <set>
#include <string>
#include <vector>
#include <sys/time.h>
#include <cmath>
#include <cstring>

#include "rknn_api.h"
#include "rknn_custom_op.h"


#include "cnpy/cnpy.h"
using namespace cnpy;

constexpr const char* golden_out_file_0 = "./model/dual_residual_output_0.bin";
constexpr const char* golden_out_file_1 = "./model/dual_residual_output_1.bin";

constexpr const size_t _PATH_MAX = 4096;

using namespace std;


/*-------------------------------------------
                  Functions
-------------------------------------------*/


static int load_bin_file(void *pBuffer, const char *fileName, size_t sizeFile)
{
  FILE *pFile = fopen(fileName, "rb");
  if (pFile == NULL) {
    puts("Error in reading files.");
    return -1;
  }

  fread(pBuffer, 1, sizeFile, pFile);

  if (fclose(pFile) != 0) {
    puts("Error in closing files.");
    return -1;
  }
  return 1;
}



static inline int64_t getCurrentTimeUs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000 + tv.tv_usec;
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

static void* load_file(const char* file_path, size_t* file_size)
{
  FILE* fp = fopen(file_path, "rb");
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


static unsigned char *load_npy(const char *input_path, rknn_tensor_attr *input_attr, int *input_type, int *input_size,
                               int *type_bytes)
{
  printf("Loading %s\n", input_path);

  NpyArray npy_data = npy_load(input_path);

  *type_bytes = npy_data.word_size;
  std::string typeName = npy_data.typeName;

  printf("npy data type:%s\n", typeName.c_str());

  if (typeName == "int8")
  {
    *input_type = RKNN_TENSOR_INT8;
  }
  else if (typeName == "uint8")
  {
    *input_type = RKNN_TENSOR_UINT8;
  }
  else if (typeName == "float16")
  {
    *input_type = RKNN_TENSOR_FLOAT16;
  }
  else if (typeName == "float32")
  {
    *input_type = RKNN_TENSOR_FLOAT32;
  }
  else if (typeName == "8")
  {
    *input_type = RKNN_TENSOR_BOOL;
  }
  else if (typeName == "int64")
  {
    *input_type = RKNN_TENSOR_INT64;
  }

  // npy shape = NHWC
  std::vector<int> npy_shape;
  for (size_t i = 0; i < npy_data.shape.size(); ++i)
  {
    npy_shape.emplace_back(npy_data.shape[i]);
  }

  int height = npy_shape.size() > 1 ? npy_shape[1] : 1;
  int width = npy_shape.size() > 2 ? npy_shape[2] : 1;
  int channel = npy_shape.size() > 3 ? npy_shape[3] : 1;

  switch (input_attr->fmt)
  {
  case RKNN_TENSOR_NHWC:
    input_attr->dims[0] = npy_shape[0];
    input_attr->dims[1] = height;
    input_attr->dims[2] = width;
    input_attr->dims[3] = channel;
    break;
  case RKNN_TENSOR_UNDEFINED:
    for (int idx = 0; idx < input_attr->n_dims; ++idx)
    {
      input_attr->dims[idx] = npy_shape[idx];
    }
    break;
  default:
    fprintf(stderr, "load_npy error, unsupport model input layout: %s\n", get_format_string(input_attr->fmt));
    break;
  }

  unsigned char *data = (unsigned char *)malloc(npy_data.num_bytes());
  if (!data)
  {
    return NULL;
  }

  // TODO: copy
  memcpy(data, npy_data.data<unsigned char>(), npy_data.num_bytes());

  *input_size = npy_data.num_bytes();

  return data;
}


double cosine(float *u, float *v, int num_elems) 
{
  double uv = 0, uu = 0, vv = 0;
  for (int i = 0; i < num_elems; i++) {
    uv += u[i] * v[i];
    uu += u[i] * u[i];
    vv += v[i] * v[i];
  }
  double cos = -1;
  if (uu != 0 && vv != 0) {
    cos = uv / sqrt(uu * vv);
  }
  return cos;
}



static void save_npy(const char* output_path, float* output_data, rknn_tensor_attr* output_attr)
{
  std::vector<size_t> output_shape;

  for (uint32_t i = 0; i < output_attr->n_dims; ++i) {
    output_shape.push_back(output_attr->dims[i]);
  }

  npy_save<float>(output_path, output_data, output_shape);
}


/**
 * float32 kernel implemetation sample for custom cpu op
 * */
int compute_custom_dual_residual_float32(rknn_custom_op_context* op_ctx, rknn_custom_op_tensor* inputs, uint32_t n_inputs,
                                    rknn_custom_op_tensor* outputs, uint32_t n_outputs)
{
  unsigned char*      in_ptr_0  = (unsigned char*)inputs[0].mem.virt_addr + inputs[0].mem.offset;
  unsigned char*      in_ptr_1   = (unsigned char*)inputs[1].mem.virt_addr + inputs[1].mem.offset;
  unsigned char*      out_ptr_0  = (unsigned char*)outputs[0].mem.virt_addr + outputs[0].mem.offset;
  unsigned char*      out_ptr_1  = (unsigned char*)outputs[1].mem.virt_addr + outputs[1].mem.offset;
  const float*        in_data_0  = (const float*)in_ptr_0;
  const float*        in_data_1  = (const float*)in_ptr_1;
  float*              out_data_0 = (float*)out_ptr_0;
  float*              out_data_1 = (float*)out_ptr_1;

  rknn_custom_op_attr op_attr;
  float alpha = 0.f;
  rknn_custom_op_get_op_attr(op_ctx, "alpha", &op_attr);
  if (op_attr.n_elems == 1 && op_attr.dtype == RKNN_TENSOR_FLOAT32) {
    alpha = ((float*)op_attr.data)[0];
  }

  const auto out_elems = outputs[0].attr.n_elems; 
  for (size_t idx=0; idx<out_elems;idx++) {
    out_data_0[idx] = in_data_0[idx] * alpha - in_data_1[idx]; 
    out_data_1[idx] = in_data_1[idx] * alpha - in_data_0[idx]; 
  }
   
  return 0;
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
  size_t model_size;
  void* model_data = load_file(model_path, &model_size);
  if (model_data == NULL) {
    return -1;
  }
  int ret = rknn_init(&ctx, model_data, model_size, 0, NULL);
  free(model_data);
  if (ret < 0) {
    printf("rknn_init fail! ret=%d\n", ret);
    return -1;
  }

  // register a custom op
  rknn_custom_op user_op[1];
  memset(user_op, 0, sizeof(rknn_custom_op));
  strncpy(user_op[0].op_type, "cstDualResidual", RKNN_MAX_NAME_LEN - 1);
  user_op[0].version = 1;
  user_op[0].target  = RKNN_TARGET_TYPE_CPU;
  user_op[0].compute = compute_custom_dual_residual_float32;
  ret = rknn_register_custom_ops(ctx, user_op, 1);
  if (ret < 0) {
      printf("rknn_register_custom_op fail! ret = %d\n", ret);
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
  int            type_bytes[io_num.n_input];
  for (int i = 0; i < io_num.n_input; i++) {
    input_data[i]   = NULL;
    input_type[i]   = RKNN_TENSOR_FLOAT32;
    input_layout[i] = input_attrs[i].fmt;
    input_size[i]   = input_attrs[i].n_elems * sizeof(float);
    type_bytes[i] = 4;
  }

  if (input_paths_split.size() > 0) {
    // Load input
    if (io_num.n_input != input_paths_split.size()) {
      printf("input missing!, need input number: %d\n", io_num.n_input);
      return -1;
    }
    for (int i = 0; i < io_num.n_input; i++) {
      // Load npy file 
      input_data[i] = load_npy(input_paths_split[i].c_str(), &input_attrs[i], &input_type[i], 
                               &input_size[i], &type_bytes[i]);

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

  //save output in npy
  for (uint32_t i = 0; i < io_num.n_output; i++) {
    char output_path[_PATH_MAX];
    sprintf(output_path, "%s/rt_output%d.npy", output_dir ? output_dir : ".", i);
    save_npy(output_path, (float*)outputs[i].buf, &output_attrs[i]);
  }

  // verify
  const size_t out_sz = 1*3*10*10*sizeof(float); 
  float *p_out_data_0 = (float*)malloc(out_sz);
  float *p_out_data_1 = (float*)malloc(out_sz);
  load_bin_file((void*)p_out_data_0, golden_out_file_0, out_sz);
  load_bin_file((void*)p_out_data_1, golden_out_file_1, out_sz);
  auto cos_out_0 = cosine( (float*)outputs[0].buf, p_out_data_0, output_attrs[0].n_elems); 
  auto cos_out_1 = cosine( (float*)outputs[1].buf, p_out_data_1, output_attrs[1].n_elems); 
  if (cos_out_0 > 0.999 && cos_out_1 > 0.999) {
    printf("Results are correct\n");
  }
  else{
    printf("Results are failed\n");
  }
  
  // release outputs
  ret = rknn_outputs_release(ctx, io_num.n_output, outputs);

  // destroy
  rknn_destroy(ctx);

  for (int i = 0; i < io_num.n_input; i++) {
    free(input_data[i]);
  }

  return 0;
}
