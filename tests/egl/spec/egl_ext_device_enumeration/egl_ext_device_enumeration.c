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

#define NDEVS 1024

int
main(void)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLint i, numdevs;
	EGLDeviceEXT devs[NDEVS];
	PFNEGLQUERYDEVICESEXTPROC queryDevices;

	const char *client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	bool has_client_ext =
		client_exts &&
		((piglit_is_extension_in_string(client_exts,
			"EGL_EXT_device_query") &&
		  piglit_is_extension_in_string(client_exts,
			"EGL_EXT_device_enumeration")) ||
		 piglit_is_extension_in_string(client_exts,
			"EGL_EXT_device_base"));

	if (!has_client_ext) {
		printf("EGL_EXT_device_enumeration not supported\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	queryDevices = (void *)eglGetProcAddress("eglQueryDevicesEXT");

	if (!queryDevices) {
		printf("No device query entrypoint\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!queryDevices(0, NULL, &numdevs)) {
		printf("Failed to get device count\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (numdevs < 1) {
		printf("No devices supported\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (numdevs > NDEVS) {
		printf("More than %d devices, please fix this test\n", NDEVS);
		result = PIGLIT_WARN;
		numdevs = NDEVS;
	}

	memset(devs, 0, sizeof devs);
	if (!queryDevices(numdevs, devs, &numdevs)) {
		printf("Failed to enumerate devices\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!numdevs) {
		printf("Zero devices enumerated\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < numdevs; i++)
	    if (devs[i] == NULL) {
		printf("Enumerated device slot not initialized\n");
		piglit_report_result(PIGLIT_FAIL);
	    }

	for (i = numdevs; i < ARRAY_SIZE(devs); i++)
	    if (devs[i] != NULL) {
		printf("Non-enumerated device slot initialized\n");
		piglit_report_result(PIGLIT_FAIL);
	    }

	queryDevices(0, devs, &numdevs);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);

	queryDevices(-1, devs, &numdevs);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);

	queryDevices(ARRAY_SIZE(devs), devs, NULL);
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(result);
}
