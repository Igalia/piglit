/*
 * Copyright © 2017 Intel Corporation
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

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

/**
 * @file egl-context-priority.c
 *
 * EGL API tests for IMG_context_priority extension:
 * https://www.khronos.org/registry/EGL/extensions/IMG/EGL_IMG_context_priority.txt
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

/* dummy */
enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static EGLint
check_priority(EGLDisplay dpy, EGLContext ctx, EGLint *expected)
{
	EGLint value;
	EGLBoolean status =
		eglQueryContext(dpy, ctx, EGL_CONTEXT_PRIORITY_LEVEL_IMG, &value);

	if (status == EGL_FALSE) {
		fprintf(stderr, "eglQueryContext failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (expected && value != *expected) {
		fprintf(stderr, "%s fail: value 0x%x, expected 0x%x\n",
			__func__, value, *expected);
		piglit_report_result(PIGLIT_FAIL);
	}
	return value;
}

static EGLContext
create_context(EGLDisplay dpy, EGLint *attr)
{
	EGLContext ctx =
		eglCreateContext(dpy, EGL_NO_CONFIG_MESA, EGL_NO_CONTEXT, attr);

	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "could not create EGL context, attr 0x%x\n", attr[1]);
		piglit_report_result(PIGLIT_FAIL);
	}
	return ctx;
}

void
piglit_init(int argc, char **argv)
{
	EGLint major, minor;
	EGLDisplay dpy;
	EGLContext ctx;
	EGLint expect;
	EGLint attr[] = { EGL_NONE, EGL_NONE, EGL_NONE };
	bool ok;

	/* Supported priority levels from extension spec. */
	EGLenum levels[] = {
		EGL_CONTEXT_PRIORITY_HIGH_IMG,
		EGL_CONTEXT_PRIORITY_MEDIUM_IMG,
		EGL_CONTEXT_PRIORITY_LOW_IMG
	};

	/* Require EGL_MESA_platform_surfaceless extension. */
	const char *exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!strstr(exts, "EGL_MESA_platform_surfaceless"))
		piglit_report_result(PIGLIT_SKIP);

	dpy = piglit_egl_get_default_display(EGL_PLATFORM_SURFACELESS_MESA);

	ok = eglInitialize(dpy, &major, &minor);
	if (!ok) {
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_egl_extension(dpy, "EGL_IMG_context_priority");
	piglit_require_egl_extension(dpy, "EGL_MESA_configless_context");

	ctx = create_context(dpy, attr);

	/* Verify default priority level. */
	expect = EGL_CONTEXT_PRIORITY_MEDIUM_IMG;
	check_priority(dpy, ctx, &expect);
	eglDestroyContext(dpy, ctx);

	/* Verify that invalid priority level fails. */
	attr[0] = EGL_CONTEXT_PRIORITY_LEVEL_IMG;
	attr[1] = EGL_TRANSPARENT_RED_VALUE;

	ctx = eglCreateContext(dpy, EGL_NO_CONFIG_MESA, EGL_NO_CONTEXT, attr);

	if (ctx != EGL_NO_CONTEXT) {
		fprintf(stderr, "should fail with invalid parameter\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Verify that creating a context with each defined priority level
	 * succeeds, also print what we asked and which priority level we got.
	 */
	for (unsigned i = 0; i < ARRAY_SIZE(levels); i++) {
		attr[1] = levels[i];
		ctx = create_context(dpy, attr);
		fprintf(stderr, "passed hint 0x%x, context created has 0x%x priority\n",
			levels[i], check_priority(dpy, ctx, NULL));

		eglDestroyContext(dpy, ctx);
	}

	piglit_report_result(PIGLIT_PASS);
}
