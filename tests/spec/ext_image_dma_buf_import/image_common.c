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

#include "image_common.h"

static void
image_common_unsupported(const char *name)
{
	printf("Function \"%s\" not supported on this implementation\n", name);
	piglit_report_result(PIGLIT_SKIP);
}

static void
resolve_eglCreateImageKHR()
{
        if (piglit_is_egl_extension_supported(eglGetCurrentDisplay(),
					"EGL_KHR_image_base"))
                image_common_dispatch_eglCreateImageKHR =
			(PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress(
				"eglCreateImageKHR");
        else
                image_common_unsupported("CreateImageKHR");
}

static EGLImageKHR EGLAPIENTRY
stub_eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target,
		EGLClientBuffer buffer, const EGLint *attrib_list)
{
        resolve_eglCreateImageKHR();
	return image_common_dispatch_eglCreateImageKHR(dpy, ctx, target, buffer,
			 attrib_list);
}

PFNEGLCREATEIMAGEKHRPROC image_common_dispatch_eglCreateImageKHR =
				stub_eglCreateImageKHR;

static void
resolve_eglDestroyImageKHR()
{
        if (piglit_is_egl_extension_supported(eglGetCurrentDisplay(),
					"EGL_KHR_image_base"))
                image_common_dispatch_eglDestroyImageKHR =
			(PFNEGLDESTROYIMAGEKHRPROC ) eglGetProcAddress(
				"eglDestroyImageKHR");
        else
                image_common_unsupported("DestroyImageKHR");
}

static EGLBoolean EGLAPIENTRY
stub_eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
        resolve_eglDestroyImageKHR();
	return image_common_dispatch_eglDestroyImageKHR(dpy, image);
}

PFNEGLDESTROYIMAGEKHRPROC image_common_dispatch_eglDestroyImageKHR =
				stub_eglDestroyImageKHR;
