/*
 * Copyright © 2016 Red Hat, Inc.
 * Copyright 2015 Intel Corporation
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

int
main(void)
{
	EGLDisplay dpy;
	EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
	EGLAttrib attr;
	const char *devstring = NULL;
	PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttrib;
	PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceString;
	PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttrib;

	const char *client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	bool has_client_ext =
		client_exts &&
		(piglit_is_extension_in_string(client_exts,
			"EGL_EXT_device_query") ||
		 piglit_is_extension_in_string(client_exts,
			"EGL_EXT_device_base"));

	if (!has_client_ext) {
		printf("EGL_EXT_device_query not supported\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	queryDisplayAttrib =
		(void *)eglGetProcAddress("eglQueryDisplayAttribEXT");
	queryDeviceString =
		(void *)eglGetProcAddress("eglQueryDeviceStringEXT");
	queryDeviceAttrib =
		(void *)eglGetProcAddress("eglQueryDeviceAttribEXT");

	if (!queryDisplayAttrib || !queryDeviceString || !queryDeviceAttrib) {
		printf("No display query entrypoint\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	dpy = eglGetDisplay(NULL);
	if (!dpy) {
		printf("failed to get EGLDisplay\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	queryDisplayAttrib(dpy, EGL_DEVICE_EXT, (EGLAttrib *)&device);
	if (!piglit_check_egl_error(EGL_NOT_INITIALIZED))
		piglit_report_result(PIGLIT_FAIL);

	if (!eglInitialize(dpy, NULL, NULL)) {
		printf("eglInitialize failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	queryDisplayAttrib(dpy, 0xbad1dea, (EGLAttrib *)&device);
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
		piglit_report_result(PIGLIT_FAIL);

	if (!queryDisplayAttrib(dpy, EGL_DEVICE_EXT, (EGLAttrib *)&device)) {
		printf("Failed to query display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (device == EGL_NO_DEVICE_EXT) {
		printf("Got no device handle\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	queryDeviceAttrib(device, 0xbad1dea, &attr);
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
		piglit_report_result(PIGLIT_FAIL);

	devstring = queryDeviceString(device, 0xbad1dea);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);

	devstring = queryDeviceString(EGL_NO_DEVICE_EXT, EGL_EXTENSIONS);
	if (!piglit_check_egl_error(EGL_BAD_DEVICE_EXT))
		piglit_report_result(PIGLIT_FAIL);

	devstring = queryDeviceString(device, EGL_EXTENSIONS);
	if (devstring == NULL) {
		printf("Empty device extension string\n");
		piglit_report_result(PIGLIT_WARN);
	}

	printf("Device extension string: %s\n", devstring);
	piglit_report_result(PIGLIT_PASS);
}
