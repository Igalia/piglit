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
 * @file egl-blob-cache.c
 *
 * EGL API tests for EGL_ANDROID_blob_cache extension:
 * https://www.khronos.org/registry/EGL/extensions/ANDROID/EGL_ANDROID_blob_cache.txt
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

/* dummy */
enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
set_blob(const void* key, EGLsizeiANDROID keySize,
	 const void* value, EGLsizeiANDROID valueSize)
{
}

static EGLsizeiANDROID
get_blob(const void* key, EGLsizeiANDROID keySize, void* value,
	 EGLsizeiANDROID valueSize)
{
	return 0;
}

void
piglit_init(int argc, char **argv)
{
	EGLint major, minor;
	EGLDisplay dpy;

	/* Require EGL_MESA_platform_surfaceless extension. */
	const char *exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!strstr(exts, "EGL_MESA_platform_surfaceless"))
		piglit_report_result(PIGLIT_SKIP);

	dpy = piglit_egl_get_default_display(EGL_PLATFORM_SURFACELESS_MESA);

	if (!eglInitialize(dpy, &major, &minor))
		piglit_report_result(PIGLIT_FAIL);

	piglit_require_egl_extension(dpy, "EGL_MESA_configless_context");
	piglit_require_egl_extension(dpy, "EGL_ANDROID_blob_cache");

	PFNEGLSETBLOBCACHEFUNCSANDROIDPROC peglSetBlobCacheFuncs =
		(PFNEGLSETBLOBCACHEFUNCSANDROIDPROC)
			eglGetProcAddress ("eglSetBlobCacheFuncsANDROID");

	if (!peglSetBlobCacheFuncs)
		piglit_report_result(PIGLIT_FAIL);

#define EXPECT(x) if (!piglit_check_egl_error(x)) piglit_report_result(PIGLIT_FAIL);

	/* Check error cases for passing NULL. */
	peglSetBlobCacheFuncs(dpy, NULL, NULL);
	EXPECT(EGL_BAD_PARAMETER);

	peglSetBlobCacheFuncs(dpy, set_blob, NULL);
	EXPECT(EGL_BAD_PARAMETER);

	peglSetBlobCacheFuncs(dpy, NULL, get_blob);
	EXPECT(EGL_BAD_PARAMETER);

	/* Successful call. */
	peglSetBlobCacheFuncs(dpy, set_blob, get_blob);
	EXPECT(EGL_SUCCESS);

	/* Failure, functions already set. */
	peglSetBlobCacheFuncs(dpy, set_blob, get_blob);
	EXPECT(EGL_BAD_PARAMETER);

#undef EXPECT

	piglit_report_result(PIGLIT_PASS);
}
