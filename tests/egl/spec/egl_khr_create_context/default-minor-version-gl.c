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
	EGLint attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 2,
		EGL_NONE
	};
	const char *version_string;
	int major;
	int minor;

	if (!EGL_KHR_create_context_setup(EGL_OPENGL_BIT)) {
		fprintf(stderr, "Desktop GL not available.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	eglBindAPI(EGL_OPENGL_API);

	/* The EGL_KHR_create_context spec says:
	 *
	 *    "Typically, the implementation will return the most recent
	 *     version of OpenGL it supports which is backwards compatible
	 *     with the requested version."
	 *
	 *     "The default values for EGL_CONTEXT_MAJOR_VERSION_KHR and
	 *     EGL_CONTEXT_MINOR_VERSION_KHR are 1 and 0 respectively."
	 *
	 * Request an OpenGL 2.0 context by explicitly setting the major
	 * version to 2 and leaving the major version at the default value of
	 * 0.  The Linux OpenGL ABI only requires OpenGL 1.2, so this might
	 * fail to create a context.
	 */
	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, attribs);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed with "
				"EGL_CONTEXT_MAJOR_VERSION_KHR=%d. skipping "
				"test.\n", attribs[1]);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	version_string = (char *) glGetString(GL_VERSION);

	if (!parse_version_string(version_string, &major, &minor)) {
		fprintf(stderr,
			"Unable to parse GL version string: %s\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	if ((major == 2 && (minor < 0 || minor > 1)) ||
	    (major == 3 && (minor != 0)) ||
	    (major < 2 || major > 3)) {
		fprintf(stderr,
			"Unexpected GL version: %s\n"
			"Expected GL 2.0, 2.1, or 3.0.\n",
			version_string);
		piglit_report_result(PIGLIT_FAIL);
	}

	eglMakeCurrent(egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(egl_dpy, ctx);

	EGL_KHR_create_context_teardown();

	piglit_report_result(PIGLIT_PASS);

	return EXIT_SUCCESS;
}
