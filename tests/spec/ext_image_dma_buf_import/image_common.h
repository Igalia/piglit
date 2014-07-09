/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef IMAGE_COMMON_H
#define IMAGE_COMMON_H

/**
 * Temporary local dispatcer for EGL extensions. This can be replaced with the
 * proper support once it is in place for EGL in the piglit core.
 *
 * A call in the tests is re-directed by the precompiler to a dispatcher that
 * checks the availability of the real extension and takes care of the linking.
 */

#include <unistd.h>
#include <drm_fourcc.h>

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

/* We define here the enums for EGL_EXT_image_dma_buf_import because, as of
 * today (2013-09-10), the eglext.h on many systems lack them.  The first Mesa
 * version in which eglext.h defined the enums was Mesa 9.2 (2013-08-27).
 */
#ifndef EGL_EXT_image_dma_buf_import
#define EGL_EXT_image_dma_buf_import 1
#define EGL_LINUX_DMA_BUF_EXT			0x3270
#define EGL_LINUX_DRM_FOURCC_EXT		0x3271
#define EGL_DMA_BUF_PLANE0_FD_EXT		0x3272
#define EGL_DMA_BUF_PLANE0_OFFSET_EXT		0x3273
#define EGL_DMA_BUF_PLANE0_PITCH_EXT		0x3274
#define EGL_DMA_BUF_PLANE1_FD_EXT		0x3275
#define EGL_DMA_BUF_PLANE1_OFFSET_EXT		0x3276
#define EGL_DMA_BUF_PLANE1_PITCH_EXT		0x3277
#define EGL_DMA_BUF_PLANE2_FD_EXT		0x3278
#define EGL_DMA_BUF_PLANE2_OFFSET_EXT		0x3279
#define EGL_DMA_BUF_PLANE2_PITCH_EXT		0x327A
#define EGL_YUV_COLOR_SPACE_HINT_EXT		0x327B
#define EGL_SAMPLE_RANGE_HINT_EXT		0x327C
#define EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT 0x327D
#define EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT 0x327E
#define EGL_ITU_REC601_EXT			0x327F
#define EGL_ITU_REC709_EXT			0x3280
#define EGL_ITU_REC2020_EXT			0x3281
#define EGL_YUV_FULL_RANGE_EXT			0x3282
#define EGL_YUV_NARROW_RANGE_EXT		0x3283
#define EGL_YUV_CHROMA_SITING_0_EXT		0x3284
#define EGL_YUV_CHROMA_SITING_0_5_EXT		0x3285
#endif

extern PFNEGLCREATEIMAGEKHRPROC image_common_dispatch_eglCreateImageKHR;
#define eglCreateImageKHR image_common_dispatch_eglCreateImageKHR

extern PFNEGLDESTROYIMAGEKHRPROC image_common_dispatch_eglDestroyImageKHR;
#define eglDestroyImageKHR image_common_dispatch_eglDestroyImageKHR

#endif /* IMAGE_COMMON_H */
