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
#ifndef PIGLIT_DRM_DMA_BUF_H
#define PIGLIT_DRM_DMA_BUF_H

#include "drm_fourcc.h"

/* These definitions were added in libdrm 2.4.68, since that's still pretty
 * recent defined these to allow older libdrm to continue working.
 */
#ifndef DRM_FORMAT_R8
#define DRM_FORMAT_R8 fourcc_code('R', '8', ' ', ' ')
#endif

#ifndef DRM_FORMAT_RG88
#define DRM_FORMAT_RG88 fourcc_code('R', 'G', '8', '8')
#endif

#ifndef DRM_FORMAT_GR88
#define DRM_FORMAT_GR88 fourcc_code('G', 'R', '8', '8')
#endif

/* added in libdrm 2.4.95 */
#ifndef DRM_FORMAT_INVALID
#define DRM_FORMAT_INVALID 0
#endif

struct piglit_dma_buf {
	unsigned w;
	unsigned h;
	unsigned offset[3];
	unsigned stride[3];
	int fd;
	void *priv;
};

enum piglit_result
piglit_drm_create_dma_buf(unsigned w, unsigned h, unsigned fourcc,
			const void *src_data,  struct piglit_dma_buf **buf);

void
piglit_drm_destroy_dma_buf(struct piglit_dma_buf *buf);

#endif /* PIGLIT_DRM_DMA_BUF_H */
