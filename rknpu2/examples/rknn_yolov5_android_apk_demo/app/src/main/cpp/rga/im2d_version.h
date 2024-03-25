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

#ifndef _RGA_IM2D_VERSION_H_
#define _RGA_IM2D_VERSION_H_

#define RGA_VERSION_STR_HELPER(x) #x
#define RGA_VERSION_STR(x) RGA_VERSION_STR_HELPER(x)

/* RGA im2d api verison */
#define RGA_API_MAJOR_VERSION       1
#define RGA_API_MINOR_VERSION       10
#define RGA_API_REVISION_VERSION    1
#define RGA_API_BUILD_VERSION       0

#define RGA_API_SUFFIX

#define RGA_API_VERSION \
    RGA_VERSION_STR(RGA_API_MAJOR_VERSION) "." \
    RGA_VERSION_STR(RGA_API_MINOR_VERSION) "." \
    RGA_VERSION_STR(RGA_API_REVISION_VERSION) "_[" \
    RGA_VERSION_STR(RGA_API_BUILD_VERSION) "]"
#define RGA_API_FULL_VERSION "rga_api version " RGA_API_VERSION RGA_API_SUFFIX

/* For header file version verification */
#define RGA_CURRENT_API_VERSION (\
    (RGA_API_MAJOR_VERSION & 0xff) << 24 | \
    (RGA_API_MINOR_VERSION & 0xff) << 16 | \
    (RGA_API_REVISION_VERSION & 0xff) << 8 | \
    (RGA_API_BUILD_VERSION & 0xff)\
    )
#define RGA_CURRENT_API_HEADER_VERSION RGA_CURRENT_API_VERSION

#endif /* _RGA_IM2D_VERSION_H_ */
