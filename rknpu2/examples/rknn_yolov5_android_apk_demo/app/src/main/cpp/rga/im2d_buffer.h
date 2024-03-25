/*
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Cerf Yu <cerf.yu@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _im2d_buffer_h_
#define _im2d_buffer_h_

#include "im2d_type.h"

/**
 * Import external buffers into RGA driver.
 *
 * @param fd/va/pa
 *      Select dma_fd/virtual_address/physical_address by buffer type
 * @param size
 *      Describes the size of the image buffer
 *
 * @return rga_buffer_handle_t
 */
#ifdef __cplusplus
IM_API rga_buffer_handle_t importbuffer_fd(int fd, int size);
IM_API rga_buffer_handle_t importbuffer_virtualaddr(void *va, int size);
IM_API rga_buffer_handle_t importbuffer_physicaladdr(uint64_t pa, int size);
#endif

/**
 * Import external buffers into RGA driver.
 *
 * @param fd/va/pa
 *      Select dma_fd/virtual_address/physical_address by buffer type
 * @param width
 *      Describes the pixel width stride of the image buffer
 * @param height
 *      Describes the pixel height stride of the image buffer
 * @param format
 *      Describes the pixel format of the image buffer
 *
 * @return rga_buffer_handle_t
 */
#ifdef __cplusplus
IM_API rga_buffer_handle_t importbuffer_fd(int fd, int width, int height, int format);
IM_API rga_buffer_handle_t importbuffer_virtualaddr(void *va, int width, int height, int format);
IM_API rga_buffer_handle_t importbuffer_physicaladdr(uint64_t pa, int width, int height, int format);
#endif

/**
 * Import external buffers into RGA driver.
 *
 * @param fd/va/pa
 *      Select dma_fd/virtual_address/physical_address by buffer type
 * @param param
 *      Configure buffer parameters
 *
 * @return rga_buffer_handle_t
 */
IM_EXPORT_API rga_buffer_handle_t importbuffer_fd(int fd, im_handle_param_t *param);
IM_EXPORT_API rga_buffer_handle_t importbuffer_virtualaddr(void *va, im_handle_param_t *param);
IM_EXPORT_API rga_buffer_handle_t importbuffer_physicaladdr(uint64_t pa, im_handle_param_t *param);

/**
 * Import external buffers into RGA driver.
 *
 * @param handle
 *      rga buffer handle
 *
 * @return success or else negative error code.
 */
IM_EXPORT_API IM_STATUS releasebuffer_handle(rga_buffer_handle_t handle);

/**
 * Wrap image Parameters.
 *
 * @param handle/virtualaddr/physicaladdr/fd
 *      RGA buffer handle/virtualaddr/physicaladdr/fd.
 * @param width
 *      Width of image manipulation area.
 * @param height
 *      Height of image manipulation area.
 * @param wstride
 *      Width pixel stride, default (width = wstride).
 * @param hstride
 *      Height pixel stride, default (height = hstride).
 * @param format
 *      Image format.
 *
 * @return rga_buffer_t
 */
#define wrapbuffer_handle(handle, width, height, format, ...) \
    ({ \
        rga_buffer_t im2d_api_buffer; \
        int __args[] = {__VA_ARGS__}; \
        int __argc = sizeof(__args)/sizeof(int); \
        if (__argc == 0) { \
            im2d_api_buffer = wrapbuffer_handle_t(handle, width, height, width, height, format); \
        } else if (__argc == 2){ \
            im2d_api_buffer = wrapbuffer_handle_t(handle, width, height, __args[0], __args[1], format); \
        } else { \
            memset(&im2d_api_buffer, 0x0, sizeof(im2d_api_buffer)); \
            printf("invalid parameter\n"); \
        } \
        im2d_api_buffer; \
    })

#define wrapbuffer_virtualaddr(vir_addr, width, height, format, ...) \
    ({ \
        rga_buffer_t im2d_api_buffer; \
        int __args[] = {__VA_ARGS__}; \
        int __argc = sizeof(__args)/sizeof(int); \
        if (__argc == 0) { \
            im2d_api_buffer = wrapbuffer_virtualaddr_t(vir_addr, width, height, width, height, format); \
        } else if (__argc == 2){ \
            im2d_api_buffer = wrapbuffer_virtualaddr_t(vir_addr, width, height, __args[0], __args[1], format); \
        } else { \
            memset(&im2d_api_buffer, 0x0, sizeof(im2d_api_buffer)); \
            printf("invalid parameter\n"); \
        } \
        im2d_api_buffer; \
    })

#define wrapbuffer_physicaladdr(phy_addr, width, height, format, ...) \
    ({ \
        rga_buffer_t im2d_api_buffer; \
        int __args[] = {__VA_ARGS__}; \
        int __argc = sizeof(__args)/sizeof(int); \
        if (__argc == 0) { \
            im2d_api_buffer = wrapbuffer_physicaladdr_t(phy_addr, width, height, width, height, format); \
        } else if (__argc == 2){ \
            im2d_api_buffer = wrapbuffer_physicaladdr_t(phy_addr, width, height, __args[0], __args[1], format); \
        } else { \
            memset(&im2d_api_buffer, 0x0, sizeof(im2d_api_buffer)); \
            printf("invalid parameter\n"); \
        } \
        im2d_api_buffer; \
    })

#define wrapbuffer_fd(fd, width, height, format, ...) \
    ({ \
        rga_buffer_t im2d_api_buffer; \
        int __args[] = {__VA_ARGS__}; \
        int __argc = sizeof(__args)/sizeof(int); \
        if (__argc == 0) { \
            im2d_api_buffer = wrapbuffer_fd_t(fd, width, height, width, height, format); \
        } else if (__argc == 2){ \
            im2d_api_buffer = wrapbuffer_fd_t(fd, width, height, __args[0], __args[1], format); \
        } else { \
            memset(&im2d_api_buffer, 0x0, sizeof(im2d_api_buffer)); \
            printf("invalid parameter\n"); \
        } \
        im2d_api_buffer; \
    })
/* Symbols for define *_t functions */
IM_C_API rga_buffer_t wrapbuffer_handle_t(rga_buffer_handle_t handle, int width, int height, int wstride, int hstride, int format);
IM_C_API rga_buffer_t wrapbuffer_virtualaddr_t(void* vir_addr, int width, int height, int wstride, int hstride, int format);
IM_C_API rga_buffer_t wrapbuffer_physicaladdr_t(void* phy_addr, int width, int height, int wstride, int hstride, int format);
IM_C_API rga_buffer_t wrapbuffer_fd_t(int fd, int width, int height, int wstride, int hstride, int format);

#ifdef __cplusplus
#undef wrapbuffer_handle
IM_API rga_buffer_t wrapbuffer_handle(rga_buffer_handle_t  handle,
                                      int width, int height, int format);
IM_API rga_buffer_t wrapbuffer_handle(rga_buffer_handle_t  handle,
                                      int width, int height, int format,
                                      int wstride, int hstride);
#endif

void imsetAlphaBit(rga_buffer_t *buf, uint8_t alpha0, uint8_t alpha1);
void imsetOpacity(rga_buffer_t *buf, uint8_t alpha);
void imsetColorSpace(rga_buffer_t *buf, IM_COLOR_SPACE_MODE mode);

#endif /* #ifndef _im2d_buffer_h_ */
