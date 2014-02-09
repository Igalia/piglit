/*
 * Copyright (c) Intel 2011
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "piglit-util-egl.h"

const char* piglit_get_egl_error_name(EGLint error) {
#define CASE(x) case x: return #x;
    switch (error) {
    CASE(EGL_SUCCESS)

    CASE(EGL_BAD_ACCESS)
    CASE(EGL_BAD_ALLOC)
    CASE(EGL_BAD_ATTRIBUTE)
    CASE(EGL_BAD_CONFIG)
    CASE(EGL_BAD_CONTEXT)
    CASE(EGL_BAD_CURRENT_SURFACE)
    CASE(EGL_BAD_DISPLAY)
    CASE(EGL_BAD_MATCH)
    CASE(EGL_BAD_NATIVE_PIXMAP)
    CASE(EGL_BAD_NATIVE_WINDOW)
    CASE(EGL_BAD_PARAMETER)
    CASE(EGL_BAD_SURFACE)
    CASE(EGL_CONTEXT_LOST)
    CASE(EGL_NOT_INITIALIZED)
    default:
        return "(unrecognized error)";
    }
#undef CASE
}

bool
piglit_check_egl_error(EGLint expected_error)
{
	EGLint actual_error;

	actual_error = eglGetError();
	if (actual_error == expected_error) {
		return true;
	}

	/*
	 * If the lookup of the error's name is successful, then print
	 *     Unexpected EGL error: NAME 0xHEX
	 * Else, print
	 *     Unexpected EGL error: 0xHEX
	 */
	printf("Unexpected EGL error: %s 0x%x\n",
               piglit_get_egl_error_name(actual_error), actual_error);

	/* Print the expected error, but only if an error was really expected. */
	if (expected_error != EGL_SUCCESS) {
		printf("Expected EGL error: %s 0x%x\n",
		piglit_get_egl_error_name(expected_error), expected_error);
        }

	return false;
}

bool
piglit_is_egl_extension_supported(EGLDisplay egl_dpy, const char *name)
{
	const char *const egl_extension_list =
		eglQueryString(egl_dpy, EGL_EXTENSIONS);

	return piglit_is_extension_in_string(egl_extension_list, name);
}

void piglit_require_egl_extension(EGLDisplay dpy, const char *name)
{
	if (!piglit_is_egl_extension_supported(dpy, name)) {
		printf("Test requires %s\n", name);
		piglit_report_result(PIGLIT_SKIP);
	}
}

bool
piglit_egl_bind_api(EGLenum api)
{
	const char *api_string = "";

	if (eglBindAPI(api))
		return true;

	if (api == EGL_OPENGL_API)
		api_string = "EGL_OPENGL_API";
	else if (api == EGL_OPENGL_ES_API)
		api_string = "EGL_OPENGL_ES_API";
	else
		assert(0);

	if (piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		fprintf(stderr, "eglBindAPI(%s) failed because EGL "
				"does not support the API\n",
			api_string);
		return false;
	} else {
		fprintf(stderr, "unexpected error for "
				"eglBindAPI(%s)\n",
				api_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	assert(0);
	return false;
}
