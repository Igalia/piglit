/* Copyright © 2012 Intel Corporation
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

#include <ctype.h>
#include <errno.h>
#include "piglit-util-egl.h"
#include "common.h"

static Display *dpy = NULL;
EGLDisplay egl_dpy;
EGLint egl_major;
EGLint egl_minor;
EGLConfig cfg;
EGLContext ctx;

bool
parse_version_string(const char *string, int *major, int *minor)
{
	char *next;
	int ma;
	int mi;

	if (string == NULL)
		return false;

	next = (char *)string;
	while (!isdigit(*next) && *next != '\0')
		next++;

	if (next[0] == '\0')
		return false;

	errno = 0;
	ma = strtol(next, &next, 10);
	if (next == NULL || errno != 0)
		return false;

	while (!isdigit(*next) && *next != '\0')
		next++;

	if (next[0] == '\0')
		return false;

	mi = strtol(next, &next, 10);
	if (errno != 0)
		return false;

	*major = ma;
	*minor = mi;
	return true;
}

static void
check_extensions(void)
{
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_create_context");
	piglit_require_egl_extension(egl_dpy, "EGL_KHR_surfaceless_context");
}

bool
EGL_KHR_create_context_setup(EGLint renderable_type_mask)
{
	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PIXMAP_BIT | EGL_PBUFFER_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_DEPTH_SIZE, 1,
		EGL_RENDERABLE_TYPE, renderable_type_mask,
		EGL_NONE
	};
	EGLint count;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	egl_dpy = eglGetDisplay(dpy);
	if (egl_dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "eglGetDisplay() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglInitialize(egl_dpy, &egl_major, &egl_minor)) {
		fprintf(stderr, "eglInitialize() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglChooseConfig(egl_dpy, config_attribs, &cfg, 1, &count) ||
	    count == 0) {
		if (eglGetError() == EGL_BAD_ATTRIBUTE) {
			/* Piglit requests only valid attributes, therefore
			 * EGL_BAD_ATTRIBUTE should not be emitted.
			 */
			fprintf(stderr, "eglChooseConfig() emitted "
			        "EGL_BAD_ATTRIBUTE\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		return false;
	}

	check_extensions();
	return true;
}

void
EGL_KHR_create_context_teardown(void)
{
	eglTerminate(egl_dpy);
}
