/*
 * Copyright © 2016 Red Hat, Inc.
 * Copyright 2015 Intel Corporation
 * Copyright 2018 Collabora, Ltd.
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
	EGLDeviceEXT devs[NDEVS];
	EGLint i, numdevs, swdevs;
	EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
	EGLAttrib attr;
	const char *devstring = NULL;
	PFNEGLQUERYDEVICESEXTPROC queryDevices;
	PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceString;
	PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttrib;

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
		printf("EGL_EXT_device_query not supported\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	queryDevices = (void *)eglGetProcAddress("eglQueryDevicesEXT");

	queryDeviceString =
		(void *)eglGetProcAddress("eglQueryDeviceStringEXT");
	queryDeviceAttrib =
		(void *)eglGetProcAddress("eglQueryDeviceAttribEXT");

	if (!queryDevices|| !queryDeviceString || !queryDeviceAttrib) {
		printf("No device query/enumeration entrypoints\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!queryDevices(0, NULL, &numdevs)) {
		printf("Failed to get device count\n");
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

	for (i = 0; i < numdevs; i++) {
		device = devs[i];
		devstring = queryDeviceString(device, EGL_EXTENSIONS);
		if (devstring == NULL) {
			printf("Empty device extension string\n");
			continue;
		}

		if (!piglit_is_extension_in_string(devstring,
					"EGL_MESA_device_software")) {
			printf("Device is not a software one\n");
			continue;
		}
		swdevs++;

		/* Extension does not define any attrib/string tokens.
		 *
		 * Double-check we don't expose claim to support other
		 * extension's tokens
		 */
		queryDeviceAttrib(device, 0xbad1dea, &attr);
		if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
			piglit_report_result(PIGLIT_FAIL);

#ifndef EGL_DRM_DEVICE_FILE_EXT
#define EGL_DRM_DEVICE_FILE_EXT                 0x3233
#endif
		devstring = queryDeviceString(device, EGL_DRM_DEVICE_FILE_EXT);
		if (!piglit_check_egl_error(EGL_BAD_PARAMETER))
			piglit_report_result(PIGLIT_FAIL);
	}

	/* SKIP if we fetched all devices with none supporting the extension */
	if (result == PIGLIT_PASS && !swdevs)
		result = PIGLIT_SKIP;

	piglit_report_result(result);
}
