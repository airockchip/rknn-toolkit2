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

#include "rknn_custom_op.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif


static rknn_custom_op user_op{};

static char*  cl_kernel_source = "#pragma OPENCL EXTENSION cl_arm_printf : enable \n"
" __kernel void Argmax_channel_float(__global const float* src_buf, const int inputChannel, __global float* dst_buf) \n"
"{\n"
" unsigned int i = get_global_id(0);\n" 
" unsigned int j = get_global_id(1); \n"
" unsigned int width = get_global_size(0);\n"
" unsigned int height = get_global_size(1);\n"
" int max_index = 0;\n"
" float max_value = src_buf[i*width+j];\n" 
" for (int c=1; c < inputChannel; c++) {\n"
"   float value = src_buf[c*height*width + i*width+j];\n"
"   if (value > max_value) {\n"
"     max_value = value;\n"
"     max_index = c;\n"
"    }\n"
"  }\n"
" dst_buf[i*width+j] = max_index;\n" 
"}\n";


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

/**
 * opencl kernel init callback for custom op
 * */
int custom_init_callback_gpu(rknn_custom_op_context* op_ctx, rknn_custom_op_tensor* inputs, uint32_t n_inputs,
                            rknn_custom_op_tensor* outputs, uint32_t n_outputs)
{
  printf("argmax_init_callback_gpu\n");
  // 获取opencl context
  cl_context cl_ctx = (cl_context)op_ctx->gpu_ctx.cl_context;

  return 0;
}

/**
 * opencl kernel init callback for custom op
 * */
int compute_custom_gpu_op_float32(rknn_custom_op_context* op_ctx, rknn_custom_op_tensor* inputs, uint32_t num_inputs,
                                 rknn_custom_op_tensor* outputs, uint32_t num_outputs)
{

  // get context
  cl_context cl_ctx = (cl_context)op_ctx->gpu_ctx.cl_context;

  // get command queue
  cl_command_queue queue = (cl_command_queue)op_ctx->gpu_ctx.cl_command_queue;

  // get kernel
  cl_kernel kernel = (cl_kernel)op_ctx->gpu_ctx.cl_kernel;

  // import input/output buffer
  const cl_import_properties_arm props[3] = {
    CL_IMPORT_TYPE_ARM,
    CL_IMPORT_TYPE_DMA_BUF_ARM,
    0,
  };

  cl_int status;
  cl_mem inObject = clImportMemoryARM(cl_ctx, CL_MEM_READ_WRITE, props, &inputs[0].mem.fd,
                                      inputs[0].mem.offset + inputs[0].mem.size, &status);
  if (status != CL_SUCCESS) {
    printf("Tensor: %s clImportMemoryARM failed\n", inputs[0].attr.name);
  }
  cl_mem outObject = clImportMemoryARM(cl_ctx, CL_MEM_READ_WRITE, props, &outputs[0].mem.fd,
                                       outputs[0].mem.offset + outputs[0].mem.size, &status);
  if (status != CL_SUCCESS) {
    printf("Tensor: %s clImportMemoryARM failed\n", outputs[0].attr.name);
  }

  int          in_type_bytes  = get_type_bytes(inputs[0].attr.type);
  int          out_type_bytes = get_type_bytes(outputs[0].attr.type);
  int          in_offset      = inputs[0].mem.offset / in_type_bytes;
  int          out_offset     = outputs[0].mem.offset / out_type_bytes;
  unsigned int elems          = inputs[0].attr.n_elems;

  auto in_channel = inputs[0].attr.dims[1]; 
  auto dst_width = outputs[0].attr.dims[3];
  auto dst_height = outputs[0].attr.dims[2];

  // set kernel args
  int argIndex = 0;
  clSetKernelArg(kernel, argIndex++, sizeof(cl_mem), &inObject);
  clSetKernelArg(kernel, argIndex++, sizeof(int), &in_channel);
  clSetKernelArg(kernel, argIndex++, sizeof(cl_mem), &outObject);

  size_t global_work_size[] = { (size_t)dst_width , (size_t)dst_height};
  auto workItems_dims = 2;

  // enqueueNDRangeKernel
  clEnqueueNDRangeKernel(queue, kernel, workItems_dims, NULL, global_work_size, NULL, 0, NULL, NULL);

  // finish command queue
  clFinish(queue);

  return 0;
}

/**
 * opencl kernel destroy callback for custom op
 */
int destroy_callback_gpu(rknn_custom_op_context* op_ctx)
{
  // clear tmp buffer
  printf("Argmax_destroy_callback_gpu\n");
  return 0;
}

//code
RKNN_CUSTOM_OP_EXPORT rknn_custom_op* get_rknn_custom_op()
{
  // register a custom op
  memset(&user_op, 0, sizeof(rknn_custom_op));
  char op_type[] = "ArgMax";
  strcpy(user_op.op_type, op_type);
  user_op.version = 1;
  user_op.target  = RKNN_TARGET_TYPE_GPU;
  user_op.init    = custom_init_callback_gpu;
  user_op.compute = compute_custom_gpu_op_float32;
  user_op.destroy = destroy_callback_gpu;

  char kernel_name[] = "Argmax_channel_float";
  strcpy(user_op.cl_kernel_name, kernel_name);

  user_op.cl_kernel_source = cl_kernel_source;
  user_op.cl_source_size = strlen(cl_kernel_source);

  return &user_op;
}

#ifdef __cplusplus
} // extern "C"
#endif
