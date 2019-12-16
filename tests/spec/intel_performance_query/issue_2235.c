/*
 * Copyright © 2019 Intel Corporation
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

/**
 * @file issue_2235.c
 * @author Lionel Landwerlin
 *
 * Reproduction for INTEL performance query assert when tearing down a
 * context with an active query.
 */

#include <time.h>

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

int main(int argc, char **argv)
{
	EGLint major, minor;
	EGLDisplay dpy;
	EGLContext ctx1;
	EGLint attr[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 2,
		EGL_NONE
	};
	GLuint query, query_handle;
	bool ok;

	dpy = piglit_egl_get_default_display(EGL_NONE);

	ok = eglInitialize(dpy, &major, &minor);
	if (!ok) {
		piglit_report_result(PIGLIT_FAIL);
		return -1;
	}

	ctx1 = eglCreateContext(dpy, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attr);

	if (!ctx1) {
		piglit_report_result(PIGLIT_FAIL);
		return -1;
	}

	dpy = piglit_egl_get_default_display(EGL_NONE);
	/*
	 * Bind first context, make some shaders, draw something.
	 */
	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx1);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_require_extension("GL_INTEL_performance_query");

	glGetFirstPerfQueryIdINTEL(&query);
	if (!query) {
		/* No query available */
		piglit_report_result(PIGLIT_SKIP);
		return 0;
	}

	glCreatePerfQueryINTEL(query, &query_handle);

	glBeginPerfQueryINTEL(query_handle);

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroyContext(dpy, ctx1);

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
