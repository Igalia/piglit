/* Copyright © 2013 Intel Corporation
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
 * \file
 *
 * This test makes current a context, terminates the context's display, then
 * unbinds the context. According to the EGL 1.4 spec (2011.04.06), Section
 * 3.2 Initialization, no error should occur.
 *
 *     EGLBoolean eglTerminate(EGLDisplay dpy);
 *
 *     Termination marks all EGL-specific resources, such as contexts and
 *     surfaces, associated with the specified display for deletion. Handles
 *     to all such resources are invalid as soon as eglTerminate returns, but
 *     the dpy handle itself remains valid. [...] Applications should not try
 *     to perform useful work with such resources following eglTerminate; only
 *     eglMakeCurrent or eglReleaseThread should be called, to complete
 *     deletion of these resources.
 *
 *     If contexts or surfaces created with respect to dpy are current (see
 *     section 3.7.3) to any thread, then they are not actually destroyed
 *     while they remain current. Such contexts and surfaces will be destroyed
 *     as soon as eglReleaseThread is called from the thread they are bound
 *     to, or eglMakeCurrent is called from that thread with the current
 *     rendering API (see section 3.7) set such that the current context is
 *     affected. [...]
 */

#include <stdbool.h>
#include <stdio.h>

#include "piglit-util-egl.h"

#define fail(msg) \
	do { \
		fprintf(stderr, "error: %s:%d: %s failed\n", __func__, __LINE__, msg); \
		piglit_report_result(PIGLIT_FAIL); \
	} while (0)

int
main(int argc, char **argv)
{
	EGLDisplay dpy;
	EGLint major_version;
	EGLint minor_version;
	EGLConfig config;
	EGLint num_configs = 0;
	EGLContext ctx;
	bool ok;

	dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY );
	if (!dpy)
		fail("eglGetDisplay(EGL_DEFAULT_DISPLAY) failed");

	ok = eglInitialize(dpy, &major_version, &minor_version);
	if (!ok)
		fail("eglInitialize() failed");

	/* This test tries to be window-system independent, and so avoids
	 * creating any EGLSurface. To call eglMakeCurrent() without a surface
	 * requires EGL_KHR_surfaceless_context.
	 */
	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_surfaceless_context"))
		piglit_report_result(PIGLIT_SKIP);

	ok = eglChooseConfig(dpy, NULL, &config, 1, &num_configs);
	if (!ok)
		fail("eglChooseConfig() failed");
	if (num_configs == 0)
		fail("eglChooseConfig() returned no configs\n");

	ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL );
	if (!ctx)
		fail("eglCreateContext() failed");

	ok = eglMakeCurrent(dpy, NULL, NULL, ctx);
	if (!ok)
		fail("eglMakeCurrent()");

	ok = eglTerminate(dpy);
	if (!ok)
	   fail("eglTerminate()");

	/* Unbind the context. */
	ok = eglMakeCurrent(dpy, NULL, NULL, NULL);
	if (!ok)
		fail("eglMakeCurrent(ctx=NULL)");

	piglit_report_result(PIGLIT_PASS);
	return 0;

}
