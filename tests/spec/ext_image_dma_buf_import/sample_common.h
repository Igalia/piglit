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
#ifndef SAMPLE_COMMON_H
#define SAMPLE_COMMON_H

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"
#include "piglit-framework-gl/piglit_drm_dma_buf.h"

/**
 * Create a dma buffer with format 'fourcc' setting the given pixels as its
 * content, create an EGL image against the buffer, bind it as texture and
 * sample it using a shader program.
 */
enum piglit_result
dma_buf_create_and_sample_32bpp(unsigned w, unsigned h,
			        int fourcc, const unsigned char *src);

enum piglit_result
egl_image_for_dma_buf_fd(struct piglit_dma_buf *buf, int fd, int fourcc, EGLImageKHR *out_img);

enum piglit_result
texture_for_egl_image(EGLImageKHR img, GLuint *out_tex);

void
sample_tex(GLuint tex, unsigned x, unsigned y, unsigned w, unsigned h);

void
usage(const char *name, const char *color_space);

#endif /* SAMPLE_COMMON_H */
