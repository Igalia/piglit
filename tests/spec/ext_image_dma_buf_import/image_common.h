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

#include "piglit-util-egl.h"
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/eglext.h>

extern PFNEGLCREATEIMAGEKHRPROC image_common_dispatch_eglCreateImageKHR;
#define eglCreateImageKHR image_common_dispatch_eglCreateImageKHR

extern PFNEGLDESTROYIMAGEKHRPROC image_common_dispatch_eglDestroyImageKHR;
#define eglDestroyImageKHR image_common_dispatch_eglDestroyImageKHR

#endif /* IMAGE_COMMON_H */
