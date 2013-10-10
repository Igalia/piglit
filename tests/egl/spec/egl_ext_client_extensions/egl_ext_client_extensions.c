/* Copyright 2013 Intel Corporation
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
 * \brief Tests for EGL_EXT_client_extensions.
 */

#include "piglit-util-egl.h"

static const char *prog_name;

static void
usage_error(void)
{
	printf("%s: usage_error\n", prog_name);
	printf("usage: %s 1|2|3\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

/**
 * Conformance test #1 in the EGL_EXT_client_extensions spec:
 *
 *     1. Before any call to eglGetDisplay, call `eglQueryString(EGL_NO_DISPLAY,
 *        EGL_EXTENSIONS)`. Verify that either
 *
 *          a. The call returns NULL and generates EGL_BAD_DISPLAY.
 *          b. The call returns an extension string that contains, at a minimum,
 *             this extension and generates no error.
 */
static void
test_1(void)
{
	enum piglit_result result = PIGLIT_PASS;
	const char *client_extensions;

	printf("Making process's first EGL call, "
	       "eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS) ...\n");
	client_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

	if (client_extensions == NULL) {
		printf("Returned NULL\n");
		if (piglit_check_egl_error(EGL_BAD_DISPLAY)) {
			printf("And correctly emitted EGL_BAD_DISPLAY\n");
		} else {
			printf("But did not emit EGL_BAD_DISPLAY\n");
			result = PIGLIT_FAIL;
		}
	} else {
		printf("Returned a non-null extension string\n");

		if (!piglit_check_egl_error(EGL_SUCCESS)) {
			result = PIGLIT_FAIL;
		}

		if (!piglit_is_extension_in_string(client_extensions,
						   "EGL_EXT_client_extensions")) {
			printf("But it does not contain "
			       "EGL_EXT_client_extensions\n");
			result = PIGLIT_FAIL;
		} else {
			printf("And contains EGL_EXT_client_extensions "
			       "as expected\n");
		}
	}

	piglit_report_result(result);
}

/**
 * Conformance test #2 in the EGL_EXT_client_extensions spec:
 *
 *    2. Obtain a display with eglGetDisplay but do not initialize it. Verify
 *       that passing the uninitialized display to `eglQueryString(dpy,
 *       EGL_EXTENSIONS)` returns NULL and generates EGL_NOT_INITIALIZED.
 */
static void
test_2(void)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay dpy;
	const char *display_extensions;

	dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!dpy) {
		printf("Failed to get default display. Skipping.n");
		piglit_report_result(PIGLIT_SKIP);
	}

	printf("Calling eglQueryString(dpy, EGL_EXTENSIONS) with an "
	       "uninitialized display...\n");
	display_extensions = eglQueryString(dpy, EGL_EXTENSIONS);

	if (display_extensions == NULL) {
		printf("Correctly returned null extension string\n");
	} else {
		printf("Did not return null extension string\n");
		result = PIGLIT_FAIL;
	}

	if (!piglit_check_egl_error(EGL_NOT_INITIALIZED)) {
		result = PIGLIT_FAIL;
	}

	piglit_report_result(result);
}

/**
 * Conformance test #3 in the EGL_EXT_client_extensions spec:
 *
 *  3. Obtain a list of display extensions by calling `eglQueryString(dpy,
 *     EGL_EXTENSIONS)` on an initialized display. Obtain the list of client
 *     extensions by calling `eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS)`.
 *     If both calls succeed, verify the two lists are disjoint.
 *
 */
static void
test_3(void)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay dpy;
	EGLint major_version, minor_version;
	const char *display_ext_string;
	const char *client_ext_string;
	const char **display_ext_array;
	const char **client_ext_array;
	int i, j;

	dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!dpy) {
		printf("Failed to get default display. Skipping.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	if (!eglInitialize(dpy, &major_version, &minor_version)) {
		printf("Failed to initialize default display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	display_ext_string = eglQueryString(dpy, EGL_EXTENSIONS);
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		printf("eglQueryString(EGL_EXTENSIONS) failed on default "
		       "display\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	if (!display_ext_string) {
		printf("eglQueryString(EGL_EXTENSIONS) returned null for "
		       "default display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	client_ext_string = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	if (!client_ext_string) {
		if (piglit_check_egl_error(EGL_BAD_DISPLAY)) {
			printf("eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS) "
			       "returned null. Skipping.\n");
			piglit_report_result(PIGLIT_SKIP);
		} else {
			printf("eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS) "
			       "returned null but did not emit "
			       "EGL_BAD_DISPLAY\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		abort();
	}

	display_ext_array = piglit_split_string_to_array(display_ext_string, " ");
	client_ext_array = piglit_split_string_to_array(client_ext_string, " ");

	/* Check that the two sets of extensions are disjoint. */
	for (i = 0; display_ext_array[i] != NULL; ++i) {
		const char *display_ext = display_ext_array[i];
		for (j = 0; client_ext_array[j] != NULL; ++j) {
			const char *client_ext = client_ext_array[j];
			if (strcmp(display_ext, client_ext) == 0) {
				printf("%s is listed both as a client and "
				       "display extension\n", display_ext);
				result = PIGLIT_FAIL;
			}
		}
	}

	piglit_report_result(result);
}

int
main(int argc, char **argv)
{
	prog_name = argv[0];

	piglit_strip_arg(&argc, argv, "-auto");
	piglit_strip_arg(&argc, argv, "-fbo");

	if (argc != 2)
		usage_error();

	if (strcmp(argv[1], "1") == 0) {
		test_1();
	} else if (strcmp(argv[1], "2") == 0) {
		test_2();
	} else if (strcmp(argv[1], "3") == 0) {
		test_3();
	} else {
		usage_error();
	}

	abort();
}
