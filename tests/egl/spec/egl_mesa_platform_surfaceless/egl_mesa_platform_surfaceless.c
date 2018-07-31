/*
 * Copyright 2016 Google
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

#include "piglit-util.h"
#include "piglit-util-egl.h"

/* Extension function pointers.
 *
 * Use prefix 'pegl' (piglit egl) instead of 'egl' to avoid collisions with
 * prototypes in eglext.h. */
EGLSurface (*peglCreatePlatformPixmapSurfaceEXT)(EGLDisplay display, EGLConfig config,
	    void *native_pixmap, const EGLint *attrib_list);
EGLSurface (*peglCreatePlatformWindowSurfaceEXT)(EGLDisplay display, EGLConfig config,
	    void *native_window, const EGLint *attrib_list);

static void
init_egl_extension_funcs(void)
{
	peglCreatePlatformPixmapSurfaceEXT = (void*)
		eglGetProcAddress("eglCreatePlatformPixmapSurfaceEXT");
	peglCreatePlatformWindowSurfaceEXT = (void*)
		eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
}

static void
test_setup(EGLDisplay *dpy)
{
	EGLint egl_major, egl_minor;

	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_MESA_platform_surfaceless");

	*dpy = piglit_egl_get_default_display(EGL_PLATFORM_SURFACELESS_MESA);
	if (*dpy == EGL_NO_DISPLAY) {
		printf("failed to get EGLDisplay\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!eglInitialize(*dpy, &egl_major, &egl_minor)) {
		printf("eglInitialize failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

static enum piglit_result
test_initialize_display(void *test_data)
{
	EGLDisplay dpy;

	test_setup(&dpy);

	eglTerminate(dpy);
	return PIGLIT_PASS;
}

/* Test that eglCreatePlatformWindowSurface fails with EGL_BAD_NATIVE_WINDOW.
 *
 * From the EGL_MESA_platform_surfaceless spec (v1):
 *
 *    eglCreatePlatformWindowSurface fails when called with a <display>
 *    that belongs to the surfaceless platform. It returns
 *    EGL_NO_SURFACE and generates EGL_BAD_NATIVE_WINDOW. The
 *    justification for this unconditional failure is that the
 *    surfaceless platform has no native windows, and therefore the
 *    <native_window> parameter is always invalid.
 */
static enum piglit_result
test_create_window(void *test_data)
{
	EGLDisplay dpy;
	EGLSurface surf;

	test_setup(&dpy);

	surf = peglCreatePlatformWindowSurfaceEXT(dpy, EGL_NO_CONFIG_KHR,
					      /*native_window*/ NULL,
					      /*attrib_list*/ NULL);
	if (surf) {
		printf("eglCreatePlatformWindowSurface incorrectly succeeded\n");
		return PIGLIT_FAIL;
	}

	if (!piglit_check_egl_error(EGL_BAD_NATIVE_WINDOW))
		return PIGLIT_FAIL;

	eglTerminate(dpy);
	return PIGLIT_PASS;
}

/* Test that eglCreatePlatformPixmapSurface fails with EGL_BAD_NATIVE_PIXMAP.
 *
 * From the EGL_MESA_platform_surfaceless spec (v1):
 *
 *    [Like eglCreatePlatformWindowSurface,] eglCreatePlatformPixmapSurface
 *    also fails when called with a <display> that belongs to the surfaceless
 *    platform.  It returns EGL_NO_SURFACE and generates
 *    EGL_BAD_NATIVE_PIXMAP.
 */
static enum piglit_result
test_create_pixmap(void *test_data)
{
	EGLDisplay dpy;
	EGLSurface surf;

	test_setup(&dpy);

	surf = peglCreatePlatformPixmapSurfaceEXT(dpy, EGL_NO_CONFIG_KHR,
					      /*native_pixmap*/ NULL,
					      /*attrib_list*/ NULL);
	if (surf) {
		printf("eglCreatePlatformPixmapSurface incorrectly succeeded\n");
		return PIGLIT_FAIL;
	}

	if (!piglit_check_egl_error(EGL_BAD_NATIVE_PIXMAP))
		return PIGLIT_FAIL;

	eglTerminate(dpy);
	return PIGLIT_PASS;
}

/* Test that eglCreatePbufferSurface succeeds if given an EGLConfig with
 * EGL_PBUFFER_BIT.
 *
 * From the EGL_MESA_platform_surfaceless spec (v1):
 *
 *   The surfaceless platform imposes no platform-specific restrictions on the
 *   creation of pbuffers, as eglCreatePbufferSurface has no native surface
 *   parameter. [...] Specifically, if the EGLDisplay advertises an EGLConfig
 *   whose EGL_SURFACE_TYPE attribute contains EGL_PBUFFER_BIT, then the
 *   EGLDisplay permits the creation of pbuffers.
 */
static enum piglit_result
test_create_pbuffer(void *test_data)
{
	EGLDisplay dpy = EGL_NO_DISPLAY;
	EGLConfig config = EGL_NO_CONFIG_KHR;
	EGLint num_configs = 9999;
	EGLSurface surf;

	const EGLint config_attrs[] = {
		EGL_SURFACE_TYPE,	EGL_PBUFFER_BIT,

		EGL_RED_SIZE,		EGL_DONT_CARE,
		EGL_GREEN_SIZE,		EGL_DONT_CARE,
		EGL_BLUE_SIZE,		EGL_DONT_CARE,
		EGL_ALPHA_SIZE,		EGL_DONT_CARE,
		EGL_DEPTH_SIZE, 	EGL_DONT_CARE,
		EGL_STENCIL_SIZE, 	EGL_DONT_CARE,

		/* This is a bitmask that selects the rendering API (such as
		 * EGL_OPENGL_BIT and EGL_OPENGL_ES2_BIT). Accept any API,
		 * because we don't care.
		 */
		EGL_RENDERABLE_TYPE, 	~0,

		EGL_NONE,
	};

	test_setup(&dpy);

	if (!eglChooseConfig(dpy, config_attrs, &config, 1, &num_configs)) {
		printf("eglChooseConfig failed\n");
		return PIGLIT_FAIL;
	}

	if (num_configs == 0) {
		printf("found no EGLConfig with EGL_PBUFFER_BIT... skip\n");
		return PIGLIT_SKIP;
	}

	surf = eglCreatePbufferSurface(dpy, config, /*attribs*/ NULL);
	if (!surf) {
		printf("eglCreatePbufferSurface failed\n");
		return PIGLIT_FAIL;
	}

	eglDestroySurface(dpy, surf);
	eglTerminate(dpy);
	return PIGLIT_PASS;
}

static const struct piglit_subtest subtests[] = {
	{ "initialize_display", "initialize_display", test_initialize_display },
	{ "create_window", "create_window", test_create_window },
	{ "create_pixmap", "create_pixmap", test_create_pixmap },
	{ "create_pbuffer", "create_pbuffer", test_create_pbuffer },
	{ 0 },
};

int
main(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_SKIP;
	const char **selected_names = NULL;
	size_t num_selected = 0;

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");

	piglit_parse_subtest_args(&argc, argv, subtests, &selected_names,
				  &num_selected);

	if (argc > 1) {
		fprintf(stderr, "usage error\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	init_egl_extension_funcs();
	result = piglit_run_selected_subtests(subtests, selected_names,
					      num_selected, result);
	piglit_report_result(result);
}
