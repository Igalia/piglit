/* Copyright 2015 Intel Corporation
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
 * Tests for EGL_KHR_get_all_proc_addresses and
 * EGL_KHR_client_get_all_proc_addresses.
 */

#include "piglit-util.h"
#include "piglit-util-egl.h"

static const char *(*myEGLQueryString)(EGLDisplay dpy, EGLenum name);

static const char *
bool_to_str(bool b)
{
	if (b)
		return "true";
	else
		return "false";
}

int
main(void)
{
	EGLDisplay dpy;
	EGLint egl_major, egl_minor;

	const char *client_exts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	bool has_client_ext =
		client_exts &&
		piglit_is_extension_in_string(client_exts,
			"EGL_KHR_client_get_all_proc_addresses");

	dpy = eglGetDisplay(NULL);
	if (!dpy) {
		printf("failed to get EGLDisplay\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!eglInitialize(dpy, &egl_major, &egl_minor)) {
		printf("eglInitialize failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	bool has_egl15 = (egl_major == 1 && egl_minor >= 5);

	const char *display_exts = eglQueryString(dpy, EGL_EXTENSIONS);
	size_t display_exts_len = strlen(display_exts);
	bool has_display_ext = piglit_is_egl_extension_supported(dpy,
				"EGL_KHR_get_all_proc_addresses");

	if (has_egl15 || has_client_ext || has_display_ext) {
		printf("eglGetProcAddress should work on core functions "
		       "because one of the following is true:\n"
		       "  EGL version >= 1.5 : %s\n"
		       "  EGL_KHR_get_all_proc_addresses: %s\n"
		       "  EGL_KHR_client_get_all_proc_addresses: %s\n"
		       "\n",
		       bool_to_str(has_egl15),
		       bool_to_str(has_display_ext),
		       bool_to_str(has_client_ext));

		/* We already know eglQueryString works because we used it
		 * above. Let's verify that it still works when called through
		 * eglGetProcAddress.
		 */
		myEGLQueryString = (void *) eglGetProcAddress("eglQueryString");
		if (!myEGLQueryString) {
			printf("eglGetProcAddress(\"eglQueryString\") failed\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		const char *display_exts2 = myEGLQueryString(dpy, EGL_EXTENSIONS);
		if (display_exts2 == NULL ||
		    strncmp(display_exts, display_exts2,
			    display_exts_len + 1) != 0) {
			printf("eglQueryString(EGL_EXTENSIONS) result differs "
			       "when called through eglGetProcAddress\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		printf("eglQueryString(EGL_EXTENSIONS) works when called "
		       "through eglGetProcAddress()\n");
	}

	if (client_exts) {
		/* From the EGL_KHR_get_proc_addresses v3 spec:
		 *
		 *     The EGL implementation must expose the name
		 *     EGL_KHR_client_get_all_proc_addresses if and only if it
		 *     exposes EGL_KHR_get_all_proc_addresses and
		 *     supports EGL_EXT_client_extensions.
		 */
		if (has_display_ext && !has_client_ext) {
			printf("EGL_KHR_get_all_proc_addresses and "
			       "EGL_EXT_client_extensions are supported but "
			       "not EGL_KHR_client_get_all_proc_addresses\n");
			piglit_report_result(PIGLIT_FAIL);
		} else if (has_client_ext && !has_display_ext) {
			printf("EGL_KHR_client_get_all_proc_addresses is "
			       "supported but not "
			       "EGL_KHR_get_all_proc_addresses\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}
