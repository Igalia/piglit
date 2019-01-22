/*
 * Copyright 2019 Intel Corporation
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

#ifndef EGL_MESA_query_driver
typedef char *(EGLAPIENTRYP PFNEGLGETDISPLAYDRIVERCONFIGPROC)(EGLDisplay dpy);
typedef const char *(EGLAPIENTRYP PFNEGLGETDISPLAYDRIVERNAMEPROC)(EGLDisplay dpy);
#endif

int
main(void)
{
	EGLDisplay egl_display;
	EGLint egl_major, egl_minor;
	const char *driver_name;
	char *driver_config;

	egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	if (!eglInitialize(egl_display, &egl_major, &egl_minor)) {
		printf("eglInitialize() failed with %s\n",
		       piglit_get_egl_error_name(eglGetError()));
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_egl_extension(egl_display, "EGL_MESA_query_driver");

	PFNEGLGETDISPLAYDRIVERNAMEPROC GetDisplayDriverName =
		(void *)eglGetProcAddress("eglGetDisplayDriverName");
	PFNEGLGETDISPLAYDRIVERCONFIGPROC GetDisplayDriverConfig =
		(void *)eglGetProcAddress("eglGetDisplayDriverConfig");

	if (!GetDisplayDriverName || !GetDisplayDriverConfig) {
		printf("Query driver entrypoints missing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	driver_name = GetDisplayDriverName(EGL_NO_DISPLAY);
	if (!piglit_check_egl_error(EGL_BAD_DISPLAY))
		piglit_report_result(PIGLIT_FAIL);

	driver_config = GetDisplayDriverConfig(EGL_NO_DISPLAY);
	if (!piglit_check_egl_error(EGL_BAD_DISPLAY)) {
		free(driver_config);
		piglit_report_result(PIGLIT_FAIL);
	}

	driver_name = GetDisplayDriverName(egl_display);
	if (!piglit_check_egl_error(EGL_SUCCESS))
		piglit_report_result(PIGLIT_FAIL);

	driver_config = GetDisplayDriverConfig(egl_display);
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		free(driver_config);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* TODO: validate the xml config against its DTD */

	printf("Driver name: %s\n", driver_name);
	printf("Driver config: %s\n", driver_config);
	free(driver_config);

	eglTerminate(egl_display);

	driver_name = GetDisplayDriverName(egl_display);
	if (!piglit_check_egl_error(EGL_NOT_INITIALIZED))
		piglit_report_result(PIGLIT_FAIL);

	driver_config = GetDisplayDriverConfig(egl_display);
	if (!piglit_check_egl_error(EGL_NOT_INITIALIZED)) {
		free(driver_config);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
