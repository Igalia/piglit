/* Copyright © 2012 Intel Corporation
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
#include "piglit-util-gl.h"
#include "common.h"

int main(int argc, char **argv)
{
	const char *version_string;
	int major;
	int minor;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_ES_BIT)) {
		fprintf(stderr, "ES 1 not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* The EGL 1.4 spec says:
	 *
	 *     "attrib list may be NULL or empty (ﬁrst attribute is EGL_NONE),
	 *     in which case all the attributes assume their default values"
	 *
	 * Specify a NULL attrib_list and expect to receive an ES 1.x context.
	 */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, NULL);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* FINISHME: Use PIGLIT_DISPATCH_ES1 when implemented. */
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	version_string = (char *) glGetString(GL_VERSION);

	if (!parse_version_string(version_string, &major, &minor)) {
		fprintf(stderr,
			"Unable to parse GL version string: %s\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (major != 1 || (minor != 0 && minor != 1)) {
		fprintf(stderr,
			"Unexpected GLES version: %s\n"
			"Expected ES 1.0 or ES 1.1.\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(egl_dpy, ctx);

	EGL_KHR_create_context_teardown();

	piglit_report_result(PIGLIT_PASS);

	return EXIT_SUCCESS;
}
