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

#ifndef _RGA_IM2D_TYPE_H_
#define _RGA_IM2D_TYPE_H_

#include <stdint.h>

#include "rga.h"

#define IM_API /* define API export as needed */

#ifdef __cplusplus
#define IM_C_API extern "C"
#define IM_EXPORT_API extern "C"
#else
#define IM_C_API
#define IM_EXPORT_API
#endif

#ifdef __cplusplus
#define DEFAULT_INITIALIZER(x) = x
#else
#define DEFAULT_INITIALIZER(x)
#endif

typedef uint32_t im_api_version_t;
typedef uint32_t im_job_handle_t;
typedef uint32_t im_ctx_id_t;
typedef uint32_t rga_buffer_handle_t;

typedef enum {
    /* Rotation */
    IM_HAL_TRANSFORM_ROT_90     = 1 << 0,
    IM_HAL_TRANSFORM_ROT_180    = 1 << 1,
    IM_HAL_TRANSFORM_ROT_270    = 1 << 2,
    IM_HAL_TRANSFORM_FLIP_H     = 1 << 3,
    IM_HAL_TRANSFORM_FLIP_V     = 1 << 4,
    IM_HAL_TRANSFORM_FLIP_H_V   = 1 << 5,
    IM_HAL_TRANSFORM_MASK       = 0x3f,

    /*
     * Blend
     * Additional blend usage, can be used with both source and target configs.
     * If none of the below is set, the default "SRC over DST" is applied.
     */
    IM_ALPHA_BLEND_SRC_OVER     = 1 << 6,     /* Default, Porter-Duff "SRC over DST" */
    IM_ALPHA_BLEND_SRC          = 1 << 7,     /* Porter-Duff "SRC" */
    IM_ALPHA_BLEND_DST          = 1 << 8,     /* Porter-Duff "DST" */
    IM_ALPHA_BLEND_SRC_IN       = 1 << 9,     /* Porter-Duff "SRC in DST" */
    IM_ALPHA_BLEND_DST_IN       = 1 << 10,    /* Porter-Duff "DST in SRC" */
    IM_ALPHA_BLEND_SRC_OUT      = 1 << 11,    /* Porter-Duff "SRC out DST" */
    IM_ALPHA_BLEND_DST_OUT      = 1 << 12,    /* Porter-Duff "DST out SRC" */
    IM_ALPHA_BLEND_DST_OVER     = 1 << 13,    /* Porter-Duff "DST over SRC" */
    IM_ALPHA_BLEND_SRC_ATOP     = 1 << 14,    /* Porter-Duff "SRC ATOP" */
    IM_ALPHA_BLEND_DST_ATOP     = 1 << 15,    /* Porter-Duff "DST ATOP" */
    IM_ALPHA_BLEND_XOR          = 1 << 16,    /* Xor */
    IM_ALPHA_BLEND_MASK         = 0x1ffc0,

    IM_ALPHA_COLORKEY_NORMAL    = 1 << 17,
    IM_ALPHA_COLORKEY_INVERTED  = 1 << 18,
    IM_ALPHA_COLORKEY_MASK      = 0x60000,

    IM_SYNC                     = 1 << 19,
    IM_CROP                     = 1 << 20,    /* Unused */
    IM_COLOR_FILL               = 1 << 21,
    IM_COLOR_PALETTE            = 1 << 22,
    IM_NN_QUANTIZE              = 1 << 23,
    IM_ROP                      = 1 << 24,
    IM_ALPHA_BLEND_PRE_MUL      = 1 << 25,
    IM_ASYNC                    = 1 << 26,
    IM_MOSAIC                   = 1 << 27,
    IM_OSD                      = 1 << 28,
    IM_PRE_INTR                 = 1 << 29,
    IM_ALPHA_BIT_MAP            = 1 << 30,
} IM_USAGE;

typedef enum {
    IM_RASTER_MODE              = 1 << 0,
    IM_AFBC16x16_MODE           = 1 << 1,
    IM_TILE8x8_MODE             = 1 << 2,
    IM_TILE4x4_MODE             = 1 << 3,
    IM_RKFBC64x4_MODE           = 1 << 4,
    IM_AFBC32x8_MODE            = 1 << 5,
    /* legacy */
    IM_FBC_MODE                 = IM_AFBC16x16_MODE,
    IM_TILE_MODE                = IM_TILE8x8_MODE,
} IM_RD_MODE;

typedef enum {
    IM_SCHEDULER_RGA3_CORE0     = 1 << 0,
    IM_SCHEDULER_RGA3_CORE1     = 1 << 1,
    IM_SCHEDULER_RGA2_CORE0     = 1 << 2,
    IM_SCHEDULER_RGA2_CORE1     = 1 << 3,
    IM_SCHEDULER_RGA3_DEFAULT   = IM_SCHEDULER_RGA3_CORE0,
    IM_SCHEDULER_RGA2_DEFAULT   = IM_SCHEDULER_RGA2_CORE0,
    IM_SCHEDULER_MASK           = 0xf,
    IM_SCHEDULER_DEFAULT        = 0,
} IM_SCHEDULER_CORE;

typedef enum {
    IM_ROP_AND                  = 0x88,
    IM_ROP_OR                   = 0xee,
    IM_ROP_NOT_DST              = 0x55,
    IM_ROP_NOT_SRC              = 0x33,
    IM_ROP_XOR                  = 0xf6,
    IM_ROP_NOT_XOR              = 0xf9,
} IM_ROP_CODE;

typedef enum {
    IM_MOSAIC_8                 = 0x0,
    IM_MOSAIC_16                = 0x1,
    IM_MOSAIC_32                = 0x2,
    IM_MOSAIC_64                = 0x3,
    IM_MOSAIC_128               = 0x4,
} IM_MOSAIC_MODE;

typedef enum {
    IM_BORDER_CONSTANT = 0,             /* iiiiii|abcdefgh|iiiiiii with some specified value 'i' */
    IM_BORDER_REFLECT = 2,              /* fedcba|abcdefgh|hgfedcb */
    IM_BORDER_WRAP = 3,                 /* cdefgh|abcdefgh|abcdefg */
} IM_BORDER_TYPE;

/* Status codes, returned by any blit function */
typedef enum {
    IM_YUV_TO_RGB_BT601_LIMIT   = 1 << 0,
    IM_YUV_TO_RGB_BT601_FULL    = 2 << 0,
    IM_YUV_TO_RGB_BT709_LIMIT   = 3 << 0,
    IM_YUV_TO_RGB_MASK          = 3 << 0,
    IM_RGB_TO_YUV_BT601_FULL    = 1 << 2,
    IM_RGB_TO_YUV_BT601_LIMIT   = 2 << 2,
    IM_RGB_TO_YUV_BT709_LIMIT   = 3 << 2,
    IM_RGB_TO_YUV_MASK          = 3 << 2,
    IM_RGB_TO_Y4                = 1 << 4,
    IM_RGB_TO_Y4_DITHER         = 2 << 4,
    IM_RGB_TO_Y1_DITHER         = 3 << 4,
    IM_Y4_MASK                  = 3 << 4,
    IM_RGB_FULL                 = 1 << 8,
    IM_RGB_CLIP                 = 2 << 8,
    IM_YUV_BT601_LIMIT_RANGE    = 3 << 8,
    IM_YUV_BT601_FULL_RANGE     = 4 << 8,
    IM_YUV_BT709_LIMIT_RANGE    = 5 << 8,
    IM_YUV_BT709_FULL_RANGE     = 6 << 8,
    IM_FULL_CSC_MASK            = 0xf << 8,
    IM_COLOR_SPACE_DEFAULT      = 0,
} IM_COLOR_SPACE_MODE;

typedef enum {
    IM_UP_SCALE,
    IM_DOWN_SCALE,
} IM_SCALE;

/* legacy */
typedef enum {
    INTER_NEAREST,
    INTER_LINEAR,
    INTER_CUBIC,
} IM_SCALE_MODE;

typedef enum {
    IM_INTERP_DEFAULT = 0,
    IM_INTERP_LINEAR = 1,
    IM_INTERP_CUBIC = 2,
    IM_INTERP_AVERAGE = 3,

    IM_INTERP_MASK = 0xf,
    IM_INTERP_HORIZ_SHIFT = 0,
    IM_INTERP_VERTI_SHIFT = 4,
    IM_INTERP_HORIZ_FLAG = 0x1 << 8,
    IM_INTERP_VERTI_FLAG = 0x1 << 9,
} IM_INTER_MODE;

typedef enum {
    IM_CONFIG_SCHEDULER_CORE,
    IM_CONFIG_PRIORITY,
    IM_CONFIG_CHECK,
} IM_CONFIG_NAME;

typedef enum {
    IM_OSD_MODE_STATISTICS      = 0x1 << 0,
    IM_OSD_MODE_AUTO_INVERT     = 0x1 << 1,
} IM_OSD_MODE;

typedef enum {
    IM_OSD_INVERT_CHANNEL_NONE          = 0x0,
    IM_OSD_INVERT_CHANNEL_Y_G           = 0x1 << 0,
    IM_OSD_INVERT_CHANNEL_C_RB          = 0x1 << 1,
    IM_OSD_INVERT_CHANNEL_ALPHA         = 0x1 << 2,
    IM_OSD_INVERT_CHANNEL_COLOR         = IM_OSD_INVERT_CHANNEL_Y_G |
                                          IM_OSD_INVERT_CHANNEL_C_RB,
    IM_OSD_INVERT_CHANNEL_BOTH          = IM_OSD_INVERT_CHANNEL_COLOR |
                                          IM_OSD_INVERT_CHANNEL_ALPHA,
} IM_OSD_INVERT_CHANNEL;

typedef enum {
    IM_OSD_FLAGS_INTERNAL = 0,
    IM_OSD_FLAGS_EXTERNAL,
} IM_OSD_FLAGS_MODE;

typedef enum {
    IM_OSD_INVERT_USE_FACTOR,
    IM_OSD_INVERT_USE_SWAP,
} IM_OSD_INVERT_MODE;

typedef enum {
    IM_OSD_BACKGROUND_DEFAULT_BRIGHT = 0,
    IM_OSD_BACKGROUND_DEFAULT_DARK,
} IM_OSD_BACKGROUND_DEFAULT;

typedef enum {
    IM_OSD_BLOCK_MODE_NORMAL = 0,
    IM_OSD_BLOCK_MODE_DIFFERENT,
} IM_OSD_BLOCK_WIDTH_MODE;

typedef enum {
    IM_OSD_MODE_HORIZONTAL,
    IM_OSD_MODE_VERTICAL,
} IM_OSD_DIRECTION;

typedef enum {
    IM_OSD_COLOR_PIXEL,
    IM_OSD_COLOR_EXTERNAL,
} IM_OSD_COLOR_MODE;

typedef enum {
    IM_INTR_READ_INTR           = 1 << 0,
    IM_INTR_READ_HOLD           = 1 << 1,
    IM_INTR_WRITE_INTR          = 1 << 2,
} IM_PRE_INTR_FLAGS;

typedef enum {
    IM_CONTEXT_NONE             = 0x0,
    IM_CONTEXT_SRC_FIX_ENABLE   = 0x1 << 0,     // Enable kernel to modify the image parameters of the channel.
    IM_CONTEXT_SRC_CACHE_INFO   = 0x1 << 1,     // It will replace the parameters in ctx with the modified parameters.
    IM_CONTEXT_SRC1_FIX_ENABLE  = 0x1 << 2,
    IM_CONTEXT_SRC1_CACHE_INFO  = 0x1 << 3,
    IM_CONTEXT_DST_FIX_ENABLE   = 0x1 << 4,
    IM_CONTEXT_DST_CACHE_INFO   = 0x1 << 5,
} IM_CONTEXT_FLAGS;

/* Get RGA basic information index */
typedef enum {
    RGA_VENDOR = 0,
    RGA_VERSION,
    RGA_MAX_INPUT,
    RGA_MAX_OUTPUT,
    RGA_BYTE_STRIDE,
    RGA_SCALE_LIMIT,
    RGA_INPUT_FORMAT,
    RGA_OUTPUT_FORMAT,
    RGA_FEATURE,
    RGA_EXPECTED,
    RGA_ALL,
} IM_INFORMATION;

/* Status codes, returned by any blit function */
typedef enum {
    IM_STATUS_NOERROR           =  2,
    IM_STATUS_SUCCESS           =  1,
    IM_STATUS_NOT_SUPPORTED     = -1,
    IM_STATUS_OUT_OF_MEMORY     = -2,
    IM_STATUS_INVALID_PARAM     = -3,
    IM_STATUS_ILLEGAL_PARAM     = -4,
    IM_STATUS_ERROR_VERSION     = -5,
    IM_STATUS_FAILED            =  0,
} IM_STATUS;

/* Rectangle definition */
typedef struct {
    int x;        /* upper-left x */
    int y;        /* upper-left y */
    int width;    /* width */
    int height;   /* height */
} im_rect;

typedef struct {
    int max;                    /* The Maximum value of the color key */
    int min;                    /* The minimum value of the color key */
} im_colorkey_range;


typedef struct im_nn {
    int scale_r;                /* scaling factor on R channal */
    int scale_g;                /* scaling factor on G channal */
    int scale_b;                /* scaling factor on B channal */
    int offset_r;               /* offset on R channal */
    int offset_g;               /* offset on G channal */
    int offset_b;               /* offset on B channal */
} im_nn_t;

/* im_info definition */
typedef struct {
    void* vir_addr;                     /* virtual address */
    void* phy_addr;                     /* physical address */
    int fd;                             /* shared fd */

    int width;                          /* width */
    int height;                         /* height */
    int wstride;                        /* wstride */
    int hstride;                        /* hstride */
    int format;                         /* format */

    int color_space_mode;               /* color_space_mode */
    union {
        int global_alpha;               /* global_alpha, the default should be 0xff */
        struct {
            uint16_t alpha0;
            uint16_t alpha1;
        } alpha_bit;                    /* alpha bit(e.g. RGBA5551), 0: alpha0, 1: alpha1 */
    };
    int rd_mode;

    /* legacy */
    int color;                          /* color, used by color fill */
    im_colorkey_range colorkey_range;   /* range value of color key */
    im_nn_t nn;
    int rop_code;

    rga_buffer_handle_t handle;         /* buffer handle */
} rga_buffer_t;

typedef struct im_color {
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
            uint8_t alpha;
        };
        uint32_t value;
    };
} im_color_t;

typedef struct im_osd_invert_factor {
    uint8_t alpha_max;
    uint8_t alpha_min;
    uint8_t yg_max;
    uint8_t yg_min;
    uint8_t crb_max;
    uint8_t crb_min;
} im_osd_invert_factor_t;

typedef struct im_osd_bpp2 {
    uint8_t  ac_swap;       // ac swap flag
                            // 0: CA
                            // 1: AC
    uint8_t  endian_swap;   // rgba2bpp endian swap
                            // 0: Big endian
                            // 1: Little endian
    im_color_t color0;
    im_color_t color1;
} im_osd_bpp2_t;

typedef struct im_osd_block {
    int width_mode;                 // normal or different
                                    //   IM_OSD_BLOCK_MODE_NORMAL
                                    //   IM_OSD_BLOCK_MODE_DIFFERENT
    union {
        int width;                  // normal_mode block width
        int width_index;            // different_mode block width index in RAM
    };

    int block_count;                // block count

    int background_config;          // background config is bright or dark
                                    //   IM_OSD_BACKGROUND_DEFAULT_BRIGHT
                                    //   IM_OSD_BACKGROUND_DEFAULT_DARK

    int direction;                  // osd block direction
                                    //   IM_OSD_MODE_HORIZONTAL
                                    //   IM_OSD_MODE_VERTICAL

    int color_mode;                 // using src1 color or config color
                                    //   IM_OSD_COLOR_PIXEL
                                    //   IM_OSD_COLOR_EXTERNAL
    im_color_t normal_color;        // config color: normal
    im_color_t invert_color;        // config color: invert
} im_osd_block_t;

typedef struct im_osd_invert {
    int invert_channel;         // invert channel config:
                                //   IM_OSD_INVERT_CHANNEL_NONE
                                //   IM_OSD_INVERT_CHANNEL_Y_G
                                //   IM_OSD_INVERT_CHANNEL_C_RB
                                //   IM_OSD_INVERT_CHANNEL_ALPHA
                                //   IM_OSD_INVERT_CHANNEL_COLOR
                                //   IM_OSD_INVERT_CHANNEL_BOTH
    int flags_mode;             // use external or inertnal RAM invert flags
                                //   IM_OSD_FLAGS_EXTERNAL
                                //   IM_OSD_FLAGS_INTERNAL
    int flags_index;            // flags index when using internal RAM invert flags

    uint64_t invert_flags;      // external invert flags
    uint64_t current_flags;     // current flags

    int invert_mode;            // invert use swap or factor
                                //   IM_OSD_INVERT_USE_FACTOR
                                //   IM_OSD_INVERT_USE_SWAP
    im_osd_invert_factor_t factor;

    int threash;
} im_osd_invert_t;

typedef struct im_osd {
    int osd_mode;                       // osd mode: statistics or auto_invert
                                        //   IM_OSD_MODE_STATISTICS
                                        //   IM_OSD_MODE_AUTO_INVERT
    im_osd_block_t block_parm;          // osd block info config

    im_osd_invert_t invert_config;

    im_osd_bpp2_t bpp2_info;
} im_osd_t;

typedef struct im_intr_config {
    uint32_t flags;

    int read_threshold;
    int write_start;
    int write_step;
} im_intr_config_t;

typedef struct im_opt {
    im_api_version_t version DEFAULT_INITIALIZER(RGA_CURRENT_API_HEADER_VERSION);

    int color;                          /* color, used by color fill */
    im_colorkey_range colorkey_range;   /* range value of color key */
    im_nn_t nn;
    int rop_code;

    int priority;
    int core;

    int mosaic_mode;

    im_osd_t osd_config;

    im_intr_config_t intr_config;

    int interp;

    char reserve[124];
} im_opt_t;

typedef struct im_handle_param {
    uint32_t width;
    uint32_t height;
    uint32_t format;
} im_handle_param_t;

#define IM_INTERP_HORIZ(x) ( ((x) & IM_INTERP_MASK) << IM_INTERP_HORIZ_SHIFT | IM_INTERP_HORIZ_FLAG )
#define IM_INTERP_VERTI(x) ( ((x) & IM_INTERP_MASK) << IM_INTERP_VERTI_SHIFT | IM_INTERP_VERTI_FLAG )
#define IM_INTERP(h,v) ( IM_INTERP_HORIZ(h) | IM_INTERP_VERTI(v) )

#endif /* _RGA_IM2D_TYPE_H_ */
