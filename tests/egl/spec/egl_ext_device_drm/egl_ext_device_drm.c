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

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "piglit-util.h"
#include "piglit-util-egl.h"

#define NDEVS 1024

int
main(void)
{
	EGLDisplay dpy1, dpy2, dpy3;
	enum piglit_result result = PIGLIT_PASS;
	EGLDeviceEXT devs[NDEVS];
	EGLint i, numdevs, drmdevs;
	EGLDeviceEXT device = EGL_NO_DEVICE_EXT;
	EGLAttrib attr;
	const char *devstring = NULL;
	PFNEGLQUERYDEVICESEXTPROC queryDevices;
	PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttrib;
	PFNEGLQUERYDEVICESTRINGEXTPROC queryDeviceString;
	PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttrib;
	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay;

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

	queryDisplayAttrib =
		(void *)eglGetProcAddress("eglQueryDisplayAttribEXT");
	queryDeviceString =
		(void *)eglGetProcAddress("eglQueryDeviceStringEXT");
	queryDeviceAttrib =
		(void *)eglGetProcAddress("eglQueryDeviceAttribEXT");

	if (!queryDevices||
	    !queryDisplayAttrib || !queryDeviceString || !queryDeviceAttrib) {
		printf("No device query/enumeration entrypoints\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	bool has_platform_dev_ext =
		client_exts &&
		(piglit_is_extension_in_string(client_exts,
			"EGL_EXT_platform_device"));

	if (has_platform_dev_ext) {
		getPlatformDisplay = (void *)eglGetProcAddress("eglGetPlatformDisplayEXT");
		if (!getPlatformDisplay) {
			has_platform_dev_ext = false;
			printf("No platform display entrypoint\n");
			result = PIGLIT_WARN;
		}
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
			result = PIGLIT_WARN;
			continue;
		}

		if (!piglit_is_extension_in_string(devstring,
					"EGL_EXT_device_drm")) {
			printf("Device is not a DRM one\n");
			continue;
		}
		drmdevs++;

		/* Extension defines only a single string token. */
		queryDeviceAttrib(device, 0xbad1dea, &attr);
		if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE))
			piglit_report_result(PIGLIT_FAIL);

#ifndef EGL_DRM_DEVICE_FILE_EXT
#define EGL_DRM_DEVICE_FILE_EXT                 0x3233
#endif
		devstring = queryDeviceString(device, EGL_DRM_DEVICE_FILE_EXT);
		if (devstring == NULL)
			piglit_report_result(PIGLIT_FAIL);

		if (!has_platform_dev_ext)
			continue;

		dpy1 = getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT,
				device, NULL);
		if (!dpy1) {
			printf("failed to get EGLDisplay\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (!eglInitialize(dpy1, NULL, NULL)) {
			printf("eglInitialize failed\n");
			piglit_report_result(PIGLIT_FAIL);
		}


		int fd = open(devstring, O_RDWR | O_CLOEXEC);
		if (fd < 0) {
			printf("Failed to open drm device file %s: %s\n",
				devstring, strerror(errno));
			piglit_report_result(PIGLIT_FAIL);
		}
#ifndef EGL_DRM_MASTER_FD_EXT
#define EGL_DRM_MASTER_FD_EXT                   0x333C
#endif
		const EGLint attr[] = { EGL_DRM_MASTER_FD_EXT, fd, EGL_NONE};

		dpy2 = getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT,
				device, attr);
		if (!dpy2) {
			printf("failed to get EGLDisplay\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* From the spec.
		 *
		 *   Calls to eglGetPlatformDeviceEXT() with the same values
		 *   for <platform> and <native_display> but distinct
		 *   EGL_DRM_MASTER_FD_EXT values will return separate EGLDisplays.
		 */
		if (dpy1 == dpy2) {
			printf("failed to provide separate displays\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		dpy3 = getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT,
				device, attr);
		if (!dpy3) {
			printf("failed to get EGLDisplay\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* Do the inverse as well - identical EGLdisplay should be
		 * returned when the FD is the same.
		 */
		if (dpy2 != dpy3) {
			printf("failed to provide identical displays\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* From the spec.
		 *
		 *   If EGL requires the use of the DRM file descriptor
		 *   beyond the duration of the call to eglGetPlatformDispay(),
		 *   it will duplicate it.
		 *
		 * Close the fd for now, if needed by eglInitialize/others, the
		 * driver will dup it.
		 */
		close(fd);

		if (!eglInitialize(dpy2, NULL, NULL)) {
			printf("eglInitialize failed\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		device = EGL_NO_DEVICE_EXT;
		if (!queryDisplayAttrib(dpy1, EGL_DEVICE_EXT, (EGLAttrib *)&device)) {
			printf("Failed to query display\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (device != devs[i]) {
			printf("Query display returns incorrect device\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		device = EGL_NO_DEVICE_EXT;
		if (!queryDisplayAttrib(dpy2, EGL_DEVICE_EXT, (EGLAttrib *)&device)) {
			printf("Failed to query display\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		if (device != devs[i]) {
			printf("Query display returns incorrect device\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		eglTerminate(dpy2);
		eglTerminate(dpy1);
	}

	/* SKIP if we fetched all devices with none supporting the extension */
	if (result == PIGLIT_PASS && !drmdevs)
		result = PIGLIT_SKIP;

	piglit_report_result(result);
}
